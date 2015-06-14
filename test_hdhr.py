#!/usr/bin/python

from silicondust import HDHR, HDHRError

devices = HDHR.discover()
if len(devices) > 0:
    try:
        #devices[0].upgrade(filename='hdhomerun3_cablecard_firmware_20150604.bin', wait=True)
        devices[0].set(item='/tuner0/vchannel', value='702')
        print 'Tuner 0 vchannel: ' + devices[0].get(item='/tuner0/vchannel')
        print 'Tuner 0 channel: ' + devices[0].get(item='/tuner0/channel')
        print 'Tuner 0 channelmap: ' + devices[0].get(item='/tuner0/channelmap')
        print 'Model: ' + devices[0].get(item='/sys/model')
        print 'HWModel: ' + devices[0].get(item='/sys/hwmodel')
    except HDHRError as sd_error:
        print 'Failure: ' + str(sd_error)

