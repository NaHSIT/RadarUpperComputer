from __future__ import annotations

import csv
import json
from pathlib import Path

import numpy as np


def write_products(request: dict, result: dict, radar) -> dict:
    output_dir = Path(request.get("output", {}).get("directory", "pyart-output")).expanduser().resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    stem = _safe_stem(result["taskId"])
    formats = request.get("output", {}).get("formats", ["json", "csv", "netcdf", "png", "cfradial"])
    products = []

    if "csv" in formats:
        path = output_dir / f"{stem}_wind_profile.csv"
        _write_csv(path, result)
        products.append(_product("wind-profile-csv", path, "text/csv"))
    if "netcdf" in formats:
        path = output_dir / f"{stem}_wind_profile.nc"
        _write_profile_netcdf(path, result)
        products.append(_product("wind-profile-cf-netcdf", path, "application/x-netcdf"))
    if "png" in formats:
        path = output_dir / f"{stem}_wind_profile.png"
        _write_png(path, result)
        products.append(_product("wind-profile-plot", path, "image/png"))
    if "cfradial" in formats:
        import pyart

        path = output_dir / f"{stem}_polar_volume.nc"
        pyart.io.write_cfradial(str(path), radar, format="NETCDF4")
        products.append(_product("polar-volume-cfradial", path, "application/x-netcdf"))

    result["products"] = products
    if "json" in formats:
        path = output_dir / f"{stem}_wind_profile.json"
        products.append(_product("wind-profile-json", path, "application/json"))
        result["products"] = products
        path.write_text(json.dumps(result, ensure_ascii=False, indent=2, allow_nan=False), encoding="utf-8")
    return result


def _write_csv(path: Path, result: dict) -> None:
    columns = ["level_index", "height_msl_m", "height_agl_m", "eastward_wind_m_s-1", "northward_wind_m_s-1",
               "wind_speed_m_s-1", "wind_from_direction_degree", "confidence_percent", "quality_flag", "quality_text"]
    with path.open("w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(columns)
        for level in result["levels"]:
            writer.writerow([level["levelIndex"], level["heightMslM"], level["heightAglM"], level["eastwardWindMps"],
                             level["northwardWindMps"], level["windSpeedMps"], level["windFromDirectionDeg"],
                             level["confidencePct"], level["qualityFlag"], level["qualityText"]])


def _write_profile_netcdf(path: Path, result: dict) -> None:
    from netCDF4 import Dataset

    levels = result["levels"]
    with Dataset(path, "w", format="NETCDF4") as dataset:
        dataset.Conventions = "CF-1.11"
        dataset.featureType = "profile"
        dataset.title = "Wind profile retrieved by Py-ART VAD"
        dataset.history = f"Created by {result['algorithm']['name']} with Py-ART {result['algorithm']['pyartVersion']}"
        dataset.createDimension("height", len(levels))
        _variable(dataset, "height", "f4", "height", [x["heightMslM"] for x in levels], "m", "height", axis="Z", positive="up")
        _variable(dataset, "eastward_wind", "f4", "height", [x["eastwardWindMps"] for x in levels], "m s-1", "eastward_wind")
        _variable(dataset, "northward_wind", "f4", "height", [x["northwardWindMps"] for x in levels], "m s-1", "northward_wind")
        _variable(dataset, "wind_speed", "f4", "height", [x["windSpeedMps"] for x in levels], "m s-1", "wind_speed")
        _variable(dataset, "wind_from_direction", "f4", "height", [x["windFromDirectionDeg"] for x in levels], "degree", "wind_from_direction")
        _variable(dataset, "retrieval_confidence", "f4", "height", [x["confidencePct"] for x in levels], "%", None)
        quality = dataset.createVariable("quality_flag", "u1", ("height",))
        quality[:] = np.asarray([x["qualityFlag"] for x in levels], dtype=np.uint8)
        quality.flag_values = np.asarray([0, 1], dtype=np.uint8)
        quality.flag_meanings = "valid insufficient_data"


def _variable(dataset, name, dtype, dimension, values, units, standard_name, **attributes):
    variable = dataset.createVariable(name, dtype, (dimension,), fill_value=np.float32(-9999.0), zlib=True)
    variable[:] = np.ma.masked_invalid(np.asarray([np.nan if value is None else value for value in values], dtype=np.float32))
    variable.units = units
    if standard_name:
        variable.standard_name = standard_name
    for key, value in attributes.items():
        setattr(variable, key, value)


def _write_png(path: Path, result: dict) -> None:
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    levels = result["levels"]
    height = np.asarray([x["heightAglM"] for x in levels], dtype=float)
    speed = np.asarray([np.nan if x["windSpeedMps"] is None else x["windSpeedMps"] for x in levels], dtype=float)
    direction = np.asarray([np.nan if x["windFromDirectionDeg"] is None else x["windFromDirectionDeg"] for x in levels], dtype=float)
    valid = np.asarray([x["qualityFlag"] == 0 for x in levels])
    fig, axes = plt.subplots(1, 2, figsize=(10.6667, 6.6667), dpi=150, sharey=True)
    fig.patch.set_facecolor("white")
    axes[0].plot(speed, height, color="#2274A5", linewidth=2.2, marker="o", markersize=3)
    axes[0].scatter(speed[~valid], height[~valid], color="#C43D32", marker="x", label="QC rejected")
    axes[0].set(xlabel="Wind speed (m s$^{-1}$)", ylabel="Height AGL (m)", title="Horizontal wind speed")
    axes[1].plot(direction, height, color="#16836B", linewidth=2.2, marker="o", markersize=3)
    axes[1].scatter(direction[~valid], height[~valid], color="#C43D32", marker="x")
    axes[1].set(xlabel="Wind-from direction (degree)", title="Meteorological direction", xlim=(0, 360))
    axes[1].set_xticks([0, 90, 180, 270, 360])
    for axis in axes:
        axis.grid(True, color="#DCE4EC", linewidth=0.8)
        axis.set_facecolor("#F8FAFC")
    fig.suptitle(f"Py-ART VAD Wind Profile | {result['observationTimeUtc']}", fontsize=13, fontweight="bold")
    fig.text(0.5, 0.02, f"Algorithm: {result['algorithm']['name']} | Valid levels: {result['summary']['validLevelCount']}/{result['summary']['levelCount']}", ha="center", fontsize=8)
    fig.tight_layout(rect=(0, 0.05, 1, 0.94))
    fig.savefig(path, facecolor="white", metadata={"Software": "RadarUpperComputer Py-ART service"})
    plt.close(fig)


def _safe_stem(value: str) -> str:
    return "".join(character if character.isalnum() or character in "-_" else "_" for character in value)[:80] or "wind_task"


def _product(product_type: str, path: Path, media_type: str) -> dict:
    return {"type": product_type, "path": str(path), "mediaType": media_type, "sizeBytes": path.stat().st_size if path.exists() else 0}

