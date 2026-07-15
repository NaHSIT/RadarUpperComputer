from __future__ import annotations

import argparse
import json
import os
import sys
import time
from pathlib import Path

os.environ.setdefault("PYART_QUIET", "1")

from .errors import RetrievalError
from .products import write_products
from .retrieval import retrieve_wind


def process(request: dict) -> dict:
    started = time.perf_counter()
    try:
        result, radar = retrieve_wind(request)
        return write_products(request, result, radar)
    except RetrievalError as exc:
        return _failure(request, exc.code, str(exc), started)
    except Exception as exc:
        return _failure(request, "E_INTERNAL", f"Unexpected algorithm error: {exc}", started)


def main() -> int:
    parser = argparse.ArgumentParser(description="RadarUpperComputer Py-ART worker")
    parser.add_argument("--request", type=Path, help="UTF-8 JSON request file; stdin is used when omitted")
    parser.add_argument("--health", action="store_true", help="print dependency health and exit")
    args = parser.parse_args()
    if args.health:
        return _health()
    try:
        text = args.request.read_text(encoding="utf-8") if args.request else sys.stdin.read()
        request = json.loads(text)
    except (OSError, json.JSONDecodeError) as exc:
        print(json.dumps(_failure({}, "E_REQUEST_JSON", str(exc), time.perf_counter()), ensure_ascii=False))
        return 2
    result = process(request)
    print(json.dumps(result, ensure_ascii=False, separators=(",", ":"), allow_nan=False))
    return 0 if result["success"] else 2


def _health() -> int:
    try:
        import pyart
        import numpy
        import netCDF4

        print(json.dumps({"available": True, "pyartVersion": pyart.__version__, "numpyVersion": numpy.__version__,
                          "netcdfVersion": netCDF4.__version__, "serviceVersion": "0.1.0"}, separators=(",", ":")))
        return 0
    except ImportError as exc:
        print(json.dumps({"available": False, "error": str(exc)}, separators=(",", ":")))
        return 3


def _failure(request: dict, code: str, message: str, started: float) -> dict:
    return {"schemaVersion": "radar.wind-profile/1.0", "taskId": str(request.get("taskId", "")), "success": False,
            "summary": {"levelCount": 0, "validLevelCount": 0, "validRatio": 0.0}, "levels": [], "products": [],
            "warnings": [], "errorCode": code, "errorMessage": message,
            "elapsedMs": round((time.perf_counter() - started) * 1000.0, 2)}


if __name__ == "__main__":
    raise SystemExit(main())
