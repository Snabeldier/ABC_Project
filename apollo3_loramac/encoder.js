function decodeUplink(input) {
  var results = [];
  var type = {};
  var payload = {};
  
  for (var i = 0; i < input.bytes.length; ) {
    switchVar = input.bytes[i + 1];
    switch (switchVar){
      case 105: 
        PAR = decode32float(new Uint8Array([input.bytes[i + 2],input.bytes[i + 3],input.bytes[i + 4],input.bytes[i + 5]]));
        PARchannel = input.bytes[i];
        results.push({PAR : PAR, Channel : PARchannel});
        i = i+6;
        break;
      case 106:
        var Timestamp = decode32Uint(new Uint8Array([input.bytes[i + 2],input.bytes[i + 3],input.bytes[i + 4],input.bytes[i + 5]]));
        var Timechannel = input.bytes[i];
        var date = new Date(Timestamp * 1000);    //given in milliseconds
        var year = date.getFullYear();
        var month = date.getMonth();
        var day = date.getDay();
        var hours = date.getHours();
        var minutes = date.getMinutes();
        var seconds = date.getSeconds();
        results.push({Time : Timestamp, Channel : Timechannel})
        results.push({Year : year, Month : month, Day : day, Hour : hours, Minute : minutes, Second : seconds})
        i = i+6;
        break;
      case 103:
        Temp = decode32float(new Uint8Array([input.bytes[i + 2],input.bytes[i + 3],input.bytes[i + 4],input.bytes[i + 5]]));
        tempchannel = input.bytes[i];
        results.push({Temperature : Temp, Channel : tempchannel})
        i = i+6;
        break;
      case 104:
        Humi = decode32float(new Uint8Array([input.bytes[i + 2],input.bytes[i + 3],input.bytes[i + 4],input.bytes[i + 5]]));
        Humichannel = input.bytes[i];
        results.push({Humidity : Humi, Channel : Humichannel})
        i = i+6;
        break;
      case 107:
        ShuntVoltage = decode32float(new Uint8Array([input.bytes[i + 2],input.bytes[i + 3],input.bytes[i + 4],input.bytes[i + 5]]));
        Shuntchannel = input.bytes[i];
        results.push({ShuntVoltage : ShuntVoltage, Channel : Shuntchannel})
        i = i+6;
        break;
      case 108:
        BusVoltage = decode32float(new Uint8Array([input.bytes[i + 2],input.bytes[i + 3],input.bytes[i + 4],input.bytes[i + 5]]));
        Buschannel = input.bytes[i];
        results.push({VoltageOnBus : BusVoltage, Channel : Buschannel})
        i = i+6;
        break;
      case 109:
        var Current = decode32float(new Uint8Array([input.bytes[i + 2],input.bytes[i + 3],input.bytes[i + 4],input.bytes[i + 5]]));
        var Currentchannel = input.bytes[i];
        results.push({Current : Current, Channel : Currentchannel})
        i = i+6;
        break;
      case 110:
        var CapVoltage = decode32float(new Uint8Array([input.bytes[i + 2],input.bytes[i + 3],input.bytes[i + 4],input.bytes[i + 5]]));
        var CapVoltagechannel = input.bytes[i];
        results.push({SuperCapVoltage : CapVoltage, Channel : CapVoltagechannel})
        i = i+6;
        break;
      default:
    }
  }
  return {
    //data: {
      //bytes: input.bytes,
      //PAR : PAR,
      //Timestamp: Timestamp,
      //ShuntVoltage: ShuntVoltage,
      //BusVoltage: BusVoltage,
      //Current: Current,
      //SuperCapVoltage: CapVoltage,
      //Temperature: Temp,
     // Humidity: Humi
    //},
    data: results,
    warnings: [],
    errors: []
  };
}

function decode32float(float32data){
    const dataView = new DataView(float32data.buffer);
    return dataView.getFloat32(0, false);
}

function decode32Uint(uint32data){
  const dataview = new DataView(uint32data.buffer);
  return dataview.getUint32(0, false);
}