from __future__ import annotations

import math
import struct
import time
import unittest

from simulation import test_server as simulator


class SimulatorProtocolTest(unittest.TestCase):
    def test_frame_crc_and_stream_parser(self):
        frame_a = simulator.build_frame(simulator.RESPONSE_SUCCESS, 7)
        frame_b = simulator.build_frame(simulator.QUERY_RADIAL_SCAN, 8, b"abc")
        stream = bytearray(b"noise" + frame_a + frame_b)
        self.assertEqual(
            simulator.extract_frames(stream),
            [(simulator.RESPONSE_SUCCESS, 7, b""), (simulator.QUERY_RADIAL_SCAN, 8, b"abc")],
        )
        self.assertEqual(stream, bytearray())

    def test_five_beam_geometry_and_projection(self):
        scan_id = 11
        started = time.monotonic()
        rays, legacy, truth = simulator.generate_scan(scan_id, started)
        self.assertEqual(len(rays), 5)
        self.assertEqual(len(legacy), 420)
        self.assertEqual(struct.unpack_from(">H", legacy, 9)[0], simulator.GATE_COUNT)

        for index, (payload, beam) in enumerate(zip(rays, simulator.BEAMS)):
            header = struct.unpack_from("<HHHBBffffffdIIII", payload)
            self.assertEqual(header[0], index)
            self.assertEqual(header[1], 5)
            self.assertEqual(header[2], simulator.GATE_COUNT)
            self.assertEqual(header[3], beam.beam_id)
            self.assertAlmostEqual(header[5], beam.azimuth_deg)
            self.assertAlmostEqual(header[6], beam.elevation_deg)
            self.assertAlmostEqual(header[11], beam.carrier_hz)
            self.assertEqual(header[13], simulator.FIELD_MASK)

            first_radial_cmps = struct.unpack_from("<h", payload, 56)[0]
            expected = simulator.radial_velocity(beam, truth)
            self.assertLess(abs(first_radial_cmps / 100.0 - expected), 0.15)

        eastward, northward, upward = truth
        self.assertTrue(math.isfinite(eastward))
        self.assertTrue(math.isfinite(northward))
        self.assertTrue(math.isfinite(upward))


if __name__ == "__main__":
    unittest.main()
