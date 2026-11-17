function UpdatePanelValues( data ) {
  // console.log(`allData ${allData.length-1}, ${allData[ allData.length-1 ].x}, ${allData[ allData.length-1 ].batteryStatus}`);

  var PanelModeValues = ["Off","V/C limited","Tracker"];
  var CSValues = {"0":"Off","2":"Fault","3":"Bulk","4":"Absorption","5":"Float","6":"Storage","7":"Equalise", "245":"Starting up", "247":"Auto equalise","252":"External control"};
  var latest = data[ data.length-1 ];

  $("#solarDatetime").text( `${latest.x} Z` );
  $("#solarPower").text( `${latest.solarPower}W` );
  $("#solarState").text( PanelModeValues[ latest.solarStatus ] );
  $("#solarVoltage").text( `${ (latest.solarVoltage).toFixed(1) }V` );

  $("#batteryState").text( CSValues[ latest.batteryStatus ] );
  $("#batteryVoltage").text( `${ (latest.batteryVoltage).toFixed(1) }V` );
  $("#batteryCurrent").text( `${ ScaleMillis( latest.batteryCurrent )}A` );

  $("#loadState").text( latest.loadStatus );
  $("#loadCurrent").text( `${ ScaleMillis( latest.loadCurrent ) }A` );

  $("#temperature").text( `${latest.temperature} \u2103` );
  $("#humidity").text( `${latest.humidity} %` );

  $("#tempMax").text( `${weatherMaxMin.temperature.max} \u2103` );
  $("#tempMin").text( `${weatherMaxMin.temperature.min} \u2103` );
  $("#humidMax").text( `${weatherMaxMin.humidity.max} %` );
  $("#humidMin").text( `${weatherMaxMin.humidity.min} %` );

  // console.log(`1 temp max ${weatherMaxMin.temperature.max}, temp min ${weatherMaxMin.temperature.min}`);
  // console.log(`1 humd max ${weatherMaxMin.humidity.max}, temp min ${weatherMaxMin.humidity.min}`);

}

function ScaleMillis( value ) {
  // returns a scaled value with prefix "m" for milli or not
  if ( Math.abs(value) < 100 ) {
    return [ `${value}m` ];
  }
  return [ ( value / 1000.0).toFixed(1) ];
}