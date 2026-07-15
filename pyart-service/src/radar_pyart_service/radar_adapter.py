from __future__ import annotations

import numpy as np
from datetime import timezone


def build_radar(validated: dict):
    import pyart

    azimuth = validated["azimuth"].astype(np.float32)
    elevation = validated["elevation"].astype(np.float32)
    ranges = validated["ranges"].astype(np.float32)
    velocity = validated["velocity"].astype(np.float32)
    mask = ~np.isfinite(velocity)
    if validated["cnr"] is not None:
        mask |= ~np.isfinite(validated["cnr"])
        mask |= validated["cnr"] < validated["min_cnr_db"]

    nrays = azimuth.size
    fields = {
        "velocity": {
            "data": np.ma.array(velocity, mask=mask),
            "units": "m s-1",
            "standard_name": "radial_velocity_of_scatterers_away_from_instrument",
            "long_name": "Doppler radial velocity",
            "coordinates": "elevation azimuth range",
            "_FillValue": np.float32(-9999.0),
        }
    }
    if validated["cnr"] is not None:
        fields["carrier_to_noise_ratio"] = {
            "data": np.ma.array(validated["cnr"].astype(np.float32), mask=~np.isfinite(validated["cnr"])),
            "units": "dB",
            "long_name": "Carrier to noise ratio",
            "coordinates": "elevation azimuth range",
            "_FillValue": np.float32(-9999.0),
        }

    timestamp = validated["timestamp"].astimezone(timezone.utc).isoformat().replace("+00:00", "Z")
    radar = pyart.core.Radar(
        time={"data": np.arange(nrays, dtype=np.float64), "units": f"seconds since {timestamp}"},
        _range={"data": ranges, "units": "m", "meters_to_center_of_first_gate": float(ranges[0]),
                "meters_between_gates": float(np.median(np.diff(ranges)))},
        fields=fields,
        metadata={"instrument_name": validated["instrument_name"], "source": "RadarUpperComputer Py-ART adapter"},
        scan_type="ppi",
        latitude={"data": np.array([validated["latitude"]], dtype=np.float64), "units": "degrees_north"},
        longitude={"data": np.array([validated["longitude"]], dtype=np.float64), "units": "degrees_east"},
        altitude={"data": np.array([validated["altitude"]], dtype=np.float64), "units": "m"},
        sweep_number={"data": np.array([0], dtype=np.int32)},
        sweep_mode={"data": np.array(["azimuth_surveillance"])},
        fixed_angle={"data": np.array([float(np.mean(elevation))], dtype=np.float32), "units": "degrees"},
        sweep_start_ray_index={"data": np.array([0], dtype=np.int32)},
        sweep_end_ray_index={"data": np.array([nrays - 1], dtype=np.int32)},
        azimuth={"data": azimuth, "units": "degrees"},
        elevation={"data": elevation, "units": "degrees"},
    )
    return radar, mask
