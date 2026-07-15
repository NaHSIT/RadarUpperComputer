from __future__ import annotations

import unittest
import tempfile
from pathlib import Path

import numpy as np

from radar_pyart_service.errors import RetrievalError
from radar_pyart_service.retrieval import retrieve_wind
from radar_pyart_service.worker import process


def synthetic_request(ray_count: int = 36) -> dict:
    gate_count = 24
    azimuth = np.arange(ray_count, dtype=float) * (360.0 / ray_count)
    elevation = np.full(ray_count, 12.0)
    ranges = np.arange(1, gate_count + 1, dtype=float) * 50.0
    radial = np.cos(np.deg2rad(elevation))[:, None] * (
        6.0 * np.sin(np.deg2rad(azimuth))[:, None]
        + 3.0 * np.cos(np.deg2rad(azimuth))[:, None]
    )
    return {
        "schemaVersion": "radar.wind-retrieval.request/1.0",
        "taskId": "unit-test",
        "taskType": "wind-retrieval",
        "observation": {
            "timestampUtc": "2026-07-13T08:00:00Z",
            "azimuthDeg": azimuth.tolist(),
            "elevationDeg": elevation.tolist(),
            "rangeM": ranges.tolist(),
            "radialVelocityMps": np.repeat(radial, gate_count, axis=1).tolist(),
            "cnrDb": np.full((ray_count, gate_count), 10.0).tolist(),
        },
        "algorithm": {"name": "pyart-vad-browning", "validRayMin": 16, "minimumCnrDb": -22.0},
        "output": {"formats": []},
    }


class RetrievalTest(unittest.TestCase):
    def test_recovers_known_horizontal_wind(self):
        result, _ = retrieve_wind(synthetic_request())
        valid = [level for level in result["levels"] if level["qualityFlag"] == 0]
        self.assertGreater(len(valid), 10)
        self.assertAlmostEqual(valid[5]["eastwardWindMps"], 6.0, places=1)
        self.assertAlmostEqual(valid[5]["northwardWindMps"], 3.0, places=1)
        self.assertAlmostEqual(valid[5]["windSpeedMps"], np.hypot(6.0, 3.0), places=1)
        self.assertAlmostEqual(valid[5]["windFromDirectionDeg"], 243.4349, places=1)

    def test_rejects_five_beam_input_for_pyart_vad(self):
        with self.assertRaises(RetrievalError) as context:
            retrieve_wind(synthetic_request(ray_count=5))
        self.assertEqual(context.exception.code, "E_INSUFFICIENT_RAYS")

    def test_writes_all_standard_products(self):
        with tempfile.TemporaryDirectory() as directory:
            request = synthetic_request()
            request["output"] = {
                "directory": directory,
                "formats": ["json", "csv", "netcdf", "png", "cfradial"],
            }
            result = process(request)
            self.assertTrue(result["success"], result.get("errorMessage"))
            self.assertEqual(len(result["products"]), 5)
            for product in result["products"]:
                path = Path(product["path"])
                self.assertTrue(path.is_file())
                self.assertGreater(path.stat().st_size, 0)


if __name__ == "__main__":
    unittest.main()
