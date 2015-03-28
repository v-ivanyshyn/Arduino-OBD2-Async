# Arduino-OBD2-Async
Based on [stanleyhuangyc/ArduinoOBD](https://github.com/stanleyhuangyc/ArduinoOBD/tree/master/libraries/OBD), with asynchronous data requesting.

There are new functions:
```
    void asyncRequest(byte pid); // send request for specific PID
    bool asyncInProgress();      // check that we already are waiting for response
    bool asyncGet(int& result);  // try to get response into 'result'
```

Typical usage:
```
    if (!obd.asyncInProgress())
        obd.asyncRequest(PID_RPM);
    int response = 0;
    if (obd.asyncGet(response))
        Serial.println(response);
```
