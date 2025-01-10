"""Example program to demonstrate how to send a multi-channel time series to
LSL."""
import sys
import getopt

import time
from random import random as rand
import math

from pylsl import StreamInfo, StreamOutlet, local_clock


def main(argv):
    srate = 100 # 100Hz for now, TODO: support 40kHz sampling rate
    name = 'BioSemi'
    type = 'EEG'
    n_channels = 8
    help_string = 'SendSineData.py -s <sampling_rate> -n <stream_name> -t <stream_type>'
    try:
        opts, args = getopt.getopt(argv, "hs:c:n:t:", longopts=["srate=", "channels=", "name=", "type"])
    except getopt.GetoptError:
        print(help_string)
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print(help_string)
            sys.exit()
        elif opt in ("-s", "--srate"):
            srate = float(arg)
        elif opt in ("-c", "--channels"):
            n_channels = int(arg)
        elif opt in ("-n", "--name"):
            name = arg
        elif opt in ("-t", "--type"):
            type = arg

    # first create a new stream info (here we set the name to BioSemi,
    # the content-type to EEG, 8 channels, 100 Hz, and float-valued data) The
    # last value would be the serial number of the device or some other more or
    # less locally unique identifier for the stream as far as available (you
    # could also omit it but interrupted connections wouldn't auto-recover)
    info = StreamInfo(name, type, n_channels, srate, 'float32', 'myuid34234')

    # next make an outlet
    outlet = StreamOutlet(info)

    print("now sending data...")
    start_time = local_clock()
    sent_samples = 0
    while True:
        elapsed_time = local_clock() - start_time
        required_samples = int(srate * elapsed_time) - sent_samples
        for sample_ix in range(required_samples):
            # Calculate the sine wave value based on time
            t = (sent_samples + sample_ix) / srate  # Current time in seconds
            # 1 Hz sine wave with 100x amplitude
            mysample = [100 * math.sin(2 * math.pi * t) for _ in range(n_channels)]
            outlet.push_sample(mysample)
        sent_samples += required_samples
        time.sleep(0.1)  # 1ms sleep for more precise timing at high sampling rate


if __name__ == '__main__':
    main(sys.argv[1:])
