#!/usr/bin/python

from silicondust import HDHR, HDHRError

devices = HDHR.discover()
if len(devices) > 0:
    try:
        devices[0].set('/tuner0/vchannel', '702')
        print 'Tuner 0 vchannel: ' + devices[0].get('/tuner0/vchannel')
        print 'Tuner 0 channel: ' + devices[0].get('/tuner0/channel')
        print 'Tuner 0 channelmap: ' + devices[0].get('/tuner0/channelmap')
        print 'Model: ' + devices[0].get('/sys/model')
        print 'HWModel: ' + devices[0].get('/sys/hwmodel')
    except HDHRError as sd_error:
        print 'Failure: ' + str(sd_error)

