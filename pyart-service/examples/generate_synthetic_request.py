from __future__ import annotations

import json
from pathlib import Path

import numpy as np


def main() -> None:
    ray_count = 36
    gate_count = 30
    azimuth = np.arange(ray_count, dtype=float) * (360.0 / ray_count)
    elevation = np.full(ray_count, 15.0)
    ranges = np.arange(1, gate_count + 1, dtype=float) * 40.0
    # Known horizontal wind components: eastward 6 m/s, northward 3 m/s.
    radial = np.cos(np.deg2rad(elevation))[:, None] * (
        6.0 * np.sin(np.deg2rad(azimuth))[:, None]
        + 3.0 * np.cos(np.deg2rad(azimuth))[:, None]
    )
    radial = np.repeat(radial, gate_count, axis=1)
    request = {
        "schemaVersion": "radar.wind-retrieval.request/1.0",
        "taskId": "synthetic-vad-001",
        "taskType": "wind-retrieval",
        "observation": {
            "timestampUtc": "2026-07-13T08:00:00Z",
            "instrumentName": "RadarUpperComputer-Synthetic",
            "latitudeDeg": 39.9042,
            "longitudeDeg": 116.4074,
            "altitudeM": 50.0,
            "azimuthDeg": azimuth.tolist(),
            "elevationDeg": elevation.tolist(),
            "rangeM": ranges.tolist(),
            "radialVelocityMps": radial.tolist(),
            "cnrDb": np.full((ray_count, gate_count), 15.0).tolist()
        },
        "algorithm": {"name": "pyart-vad-browning", "validRayMin": 16, "minimumCnrDb": -22.0},
        "output": {"directory": "pyart-output/synthetic-vad-001", "formats": ["json", "csv", "netcdf", "png", "cfradial"]}
    }
    output = Path(__file__).with_name("synthetic_vad_request.json")
    output.write_text(json.dumps(request, ensure_ascii=False, indent=2), encoding="utf-8")
    print(output)


if __name__ == "__main__":
    main()

