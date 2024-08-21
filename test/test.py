import random
import sys
import os
# Ensure the generated SWIG module is in the Python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'RD53py')))

from RD53py import QuarterCore, RD53Decoder, Rd53StreamConfig, RD53Event, N_QCORES_HORIZONTAL, N_QCORES_VERTICAL, RD53Header, HitCoord

if __name__ == "__main__":
    try:
        conf = Rd53StreamConfig()

        conf.chip_id = True
        conf.l1id = True
        conf.bcid = True
        conf.drop_tot = False
        conf.compressed_hitmap = True
        conf.size_qcore_vertical = 2
        conf.size_qcore_horizontal = 8

        hits = {}

        for i in range(10):
            x = random.randint(0, N_QCORES_HORIZONTAL * conf.size_qcore_horizontal - 1)
            y = random.randint(0, N_QCORES_VERTICAL * conf.size_qcore_vertical - 1)
            tot = random.randint(0, 15)

            hits[(x, y)] = tot
            # input_hits.append(HitCoord(x, y, tot))

        input_hits = [HitCoord(x, y, hits[(x, y)]) for (x, y) in hits]

        header = RD53Header()

        header.bcid = 1
        header.l1id = 56
        header.trigger_tag = 1
        header.trigger_pos = 2
        header.chip_id = 2

        event = RD53Event(conf, header, input_hits)

        encoded = event.serialize_event()
        decoder = RD53Decoder(conf, encoded)

        decoder.process_stream()

        events = decoder.get_events()

        hit_decoded = events[0].get_hits()

        print(hit_decoded)

        hit_decoded = {(hit.x, hit.y): hit.val for hit in hit_decoded}
        input_hits = {(hit.x, hit.y): hit.val for hit in input_hits}

        assert hit_decoded == input_hits

    except AssertionError as e:
        print(f"Test failed: {e}")
        sys.exit(1)
