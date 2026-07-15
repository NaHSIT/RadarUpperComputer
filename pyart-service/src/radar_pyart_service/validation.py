from __future__ import annotations

from datetime import datetime
from typing import Any

import numpy as np

from .errors import RetrievalError


SCHEMA_VERSION = "radar.wind-retrieval.request/1.0"


def validate_request(request: dict[str, Any]) -> dict[str, Any]:
    if request.get("schemaVersion") != SCHEMA_VERSION:
        raise RetrievalError("E_SCHEMA_VERSION", f"schemaVersion must be {SCHEMA_VERSION}")
    if not str(request.get("taskId", "")).strip():
        raise RetrievalError("E_TASK_ID", "taskId is required")
    if request.get("taskType") != "wind-retrieval":
        raise RetrievalError("E_TASK_TYPE", "taskType must be wind-retrieval")

    observation = request.get("observation")
    if not isinstance(observation, dict):
        raise RetrievalError("E_OBSERVATION", "observation must be an object")

    try:
        timestamp = datetime.fromisoformat(str(observation["timestampUtc"]).replace("Z", "+00:00"))
        azimuth = np.asarray(observation["azimuthDeg"], dtype=np.float64)
        elevation = np.asarray(observation["elevationDeg"], dtype=np.float64)
        ranges = np.asarray(observation["rangeM"], dtype=np.float64)
        velocity = np.asarray(observation["radialVelocityMps"], dtype=np.float64)
    except (KeyError, TypeError, ValueError) as exc:
        raise RetrievalError("E_REQUIRED_FIELD", f"invalid observation field: {exc}") from exc

    if timestamp.tzinfo is None:
        raise RetrievalError("E_TIMESTAMP", "timestampUtc must include a UTC offset")
    if azimuth.ndim != 1 or elevation.ndim != 1 or ranges.ndim != 1 or velocity.ndim != 2:
        raise RetrievalError("E_DIMENSION", "angles/range must be 1-D and radialVelocityMps must be 2-D")
    if velocity.shape != (azimuth.size, ranges.size) or elevation.size != azimuth.size:
        raise RetrievalError("E_SHAPE", "radialVelocityMps shape must be [rayCount][gateCount]")
    if azimuth.size < 16:
        raise RetrievalError("E_INSUFFICIENT_RAYS", "Py-ART VAD requires at least 16 azimuth rays")
    if ranges.size < 2 or np.any(~np.isfinite(ranges)) or np.any(np.diff(ranges) <= 0):
        raise RetrievalError("E_RANGE", "rangeM must contain increasing finite gate centers")
    if np.any(~np.isfinite(azimuth)) or np.any(~np.isfinite(elevation)):
        raise RetrievalError("E_ANGLE", "azimuthDeg and elevationDeg must be finite")
    if float(np.ptp(elevation)) > 0.5:
        raise RetrievalError("E_SWEEP_GEOMETRY", "Browning VAD requires one fixed-elevation sweep")

    cnr = observation.get("cnrDb")
    cnr_array = None if cnr is None else np.asarray(cnr, dtype=np.float64)
    if cnr_array is not None and cnr_array.shape != velocity.shape:
        raise RetrievalError("E_CNR_SHAPE", "cnrDb shape must match radialVelocityMps")

    config = request.get("algorithm", {})
    if config.get("name", "pyart-vad-browning") != "pyart-vad-browning":
        raise RetrievalError("E_ALGORITHM", "only pyart-vad-browning is supported")
    valid_ray_min = int(config.get("validRayMin", 16))
    if valid_ray_min < 16 or valid_ray_min > azimuth.size:
        raise RetrievalError("E_VALID_RAY_MIN", "validRayMin must be between 16 and rayCount")

    return {
        "timestamp": timestamp,
        "azimuth": np.mod(azimuth, 360.0),
        "elevation": elevation,
        "ranges": ranges,
        "velocity": velocity,
        "cnr": cnr_array,
        "valid_ray_min": valid_ray_min,
        "min_cnr_db": float(config.get("minimumCnrDb", -22.0)),
        "latitude": float(observation.get("latitudeDeg", 0.0)),
        "longitude": float(observation.get("longitudeDeg", 0.0)),
        "altitude": float(observation.get("altitudeM", 0.0)),
        "instrument_name": str(observation.get("instrumentName", "wind-radar")),
    }

