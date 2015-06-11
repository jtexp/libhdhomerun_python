#!/usr/bin/python

from silicondust import HDHR, HDHRError

devices = HDHR.discover()
if len(devices) > 0:
    try:
        print 'Tuner 0 channel: ' + devices[0].get('/tuner0/channel')
        print 'Tuner 0 channelmap: ' + devices[0].get('/tuner0/channelmap')
        print 'Model: ' + devices[0].get('/sys/model')
        print 'HWModel: ' + devices[0].get('/sys/hwmodel')
    except HDHRError as sd_error:
        print 'Failure: ' + str(sd_error)

