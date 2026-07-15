from __future__ import annotations

import time
import contextlib
import io
from datetime import timezone
from typing import Any

import numpy as np

from .errors import RetrievalError
from .radar_adapter import build_radar
from .validation import validate_request


def retrieve_wind(request: dict[str, Any]) -> tuple[dict[str, Any], object]:
    started = time.perf_counter()
    validated = validate_request(request)
    try:
        import pyart
    except ImportError as exc:
        raise RetrievalError("E_PYART_UNAVAILABLE", "Py-ART is not installed") from exc

    radar, excluded = build_radar(validated)
    gate_filter = pyart.filters.GateFilter(radar)
    gate_filter.exclude_masked("velocity")

    elevation_rad = np.deg2rad(float(np.mean(validated["elevation"])))
    height_agl = validated["ranges"] * np.sin(elevation_rad)
    z_want = height_agl + validated["altitude"]
    try:
        with contextlib.redirect_stdout(io.StringIO()):
            vad = pyart.retrieve.vad_browning(
                radar,
                "velocity",
                z_want=z_want,
                valid_ray_min=validated["valid_ray_min"],
                gatefilter=gate_filter,
                window=1,
                weight="equal",
            )
    except (ValueError, RuntimeError, FloatingPointError) as exc:
        raise RetrievalError("E_RETRIEVAL_FAILED", f"Py-ART VAD failed: {exc}") from exc

    heights = np.asarray(vad.height, dtype=np.float64)
    speed = np.asarray(vad.speed, dtype=np.float64)
    direction = np.mod(np.asarray(vad.direction, dtype=np.float64), 360.0)
    u_wind = np.asarray(vad.u_wind, dtype=np.float64)
    v_wind = np.asarray(vad.v_wind, dtype=np.float64)
    valid_fraction_by_gate = 1.0 - np.mean(excluded, axis=0)
    confidence = np.interp(heights, z_want, valid_fraction_by_gate, left=0.0, right=0.0) * 100.0
    finite = np.isfinite(speed) & np.isfinite(direction) & np.isfinite(u_wind) & np.isfinite(v_wind)

    levels = []
    for index in range(heights.size):
        is_valid = bool(finite[index] and confidence[index] >= 50.0)
        levels.append({
            "levelIndex": index,
            "heightMslM": _number(heights[index]),
            "heightAglM": _number(heights[index] - validated["altitude"]),
            "eastwardWindMps": _number(u_wind[index]),
            "northwardWindMps": _number(v_wind[index]),
            "windSpeedMps": _number(speed[index]),
            "windFromDirectionDeg": _number(direction[index]),
            "confidencePct": round(float(confidence[index]), 2),
            "qualityFlag": 0 if is_valid else 1,
            "qualityText": "valid" if is_valid else "insufficient-data",
        })

    valid_count = sum(level["qualityFlag"] == 0 for level in levels)
    result = {
        "schemaVersion": "radar.wind-profile/1.0",
        "taskId": request["taskId"],
        "success": valid_count > 0,
        "algorithm": {"name": "pyart-vad-browning", "pyartVersion": pyart.__version__, "validRayMin": validated["valid_ray_min"]},
        "observationTimeUtc": validated["timestamp"].astimezone(timezone.utc).isoformat().replace("+00:00", "Z"),
        "coordinateReference": {"verticalDatum": "MSL", "directionConvention": "meteorological-wind-from-clockwise-from-true-north"},
        "levels": levels,
        "summary": {"levelCount": len(levels), "validLevelCount": valid_count, "validRatio": round(valid_count / max(1, len(levels)), 4)},
        "products": [],
        "warnings": [] if valid_count == len(levels) else ["Some levels did not pass quality control."],
        "errorCode": "" if valid_count > 0 else "E_NO_VALID_LEVEL",
        "elapsedMs": round((time.perf_counter() - started) * 1000.0, 2),
    }
    return result, radar


def _number(value: float):
    return round(float(value), 6) if np.isfinite(value) else None
