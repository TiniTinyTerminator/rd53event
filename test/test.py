import sys
import os
# Ensure the generated SWIG module is in the Python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build')))

import RD53py

if __name__ == "__main__":
    try:

        RD53py.say_hello_world("YEEET")

        conf = RD53py.Rd53StreamConfig()

        conf.chip_id = True
        conf.lcid = True
        conf.bcid = True
        conf.drop_tot = False
        conf.compressed_hitmap = True
        conf.size_qcore_vertical = 2
        conf.size_qcore_horizontal = 8

        qcore = RD53py.QuarterCore(conf, 2, 5)

        qcore.set_hit_by_index(3, 12)

        print(qcore.col, qcore.row, qcore.is_last, qcore.is_neighbour, qcore.is_last_in_event, qcore.serialize_qcore(True))

        print("All tests passed.")
    except AssertionError as e:
        print(f"Test failed: {e}")
        sys.exit(1)
