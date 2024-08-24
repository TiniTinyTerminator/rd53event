from copy import deepcopy
import random
import sys
import os
# Ensure the generated SWIG module is in the Python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'RD53BEvent')))

from RD53BEvent import * #QuarterCore, Decoder, StreamConfig, Event, N_QCORES_HORIZONTAL, N_QCORES_VERTICAL, Header, HitCoord
import RD53BEvent

if __name__ == "__main__":
    try:
        conf = StreamConfig()

        conf.chip_id = True
        conf.l1id = True
        conf.bcid = True
        conf.drop_tot = False
        conf.compressed_hitmap = True
        conf.size_qcore_vertical = 2
        conf.size_qcore_horizontal = 8

        hits = {}

        for i in range(100):
            x = random.randint(0, N_QCORES_HORIZONTAL * conf.size_qcore_horizontal - 1)
            y = random.randint(0, N_QCORES_VERTICAL * conf.size_qcore_vertical - 1)
            tot = random.randint(0, 15)

            hits[(x, y)] = tot
            # input_hits.append(HitCoord(x, y, tot))

        # vec = HitCoordVector()

        # vec.

        input_hits = [(x, y, hits[(x, y)]) for (x, y) in hits]

        header = StreamHeader()

        header.bcid = 1
        header.l1id = 56
        header.trigger_tag = 1
        header.trigger_pos = 2
        header.chip_id = 2

        header2 = StreamHeader()

        header2 = header

        print(header, header2)

        event = Event(conf, header, input_hits)

        qcores = event.get_qcores()

        print("hello")

        # print(qcores)


        qcores = RD53BEvent.QcoreVector(qcores)

        print(qcores)

        other_event = Event(conf,header2,qcores)

        print(other_event.get_hits())

        encoded = event.serialize_event()

        # print(encoded)
        decoder = Decoder(conf, encoded)



        decoder.process_stream()

        events = decoder.get_events()

        hit_decoded = events[0].get_hits()

        hit_decoded = {(x, y): val for (x, y, val) in hit_decoded}
        input_hits = {(x, y): val for (x, y, val) in input_hits}

        # print(input_hits)

        assert hit_decoded == input_hits

    except AssertionError as e:
        print(f"Test failed: {e}")
        sys.exit(1)
