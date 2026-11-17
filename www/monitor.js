// field description and examples
// CS:"5"      - state of operation, off, fault, bulk, etc
// ERR:"0"     - error code
// FW:"175"    - firmware version
// H19:"15"    - yield total in 0.01 kWh
// H20:"1"     - yield today in 0.01 kWh
// H21:"8"     - maximum power today W
// H22:"0"     - yield yesterday 0.01 kWh
// H23:"9"     - maximum power yesterday W
// HSDS:"10"   - day sequence number
// I:"110"     - battery current I
// IL:"0"      - load current mA
// LOAD:"ON"   - load output state
// MPPT:"1"    - tracker operation mode, off, voltage or current limited, MPP tracker active
// PPV:"2"     - panel power W
// V:"13840"   - battery voltage mV
// VPV:"33740" - panel voltage mV
// OR:"0X00000000"    - off reason, no input power, switched off, etc
// SERN:"HQ260994ACG" - serial number
// PID:"0XA075"       - product id
// dateTime:"2026-10-03 09:07:22"
// messageType:"$V"

"use strict"
// number of data items to get is 60 * 24 * number per minute * 1.1
var localURL = 'http://localhost/getLog.php?number=3000&type=$V';
var liveURL = 'https://solar.mycarda.info/getLog.php?number=3000&type=$V';
var useURL = liveURL;


var startDate;
var allData = [];

function initialise()
{
  let urlParams = new URLSearchParams(window.location.search);
  // console.log(`Victron js init`);
  InitialiseCharts()
  GetStats();

  setInterval( GetStats, 60000);
}

function OneDayAgo() {
  var yesterday = new Date(new Date().getTime() - (24 * 60 * 60 * 1000));
  // console.log(`yesterday ${yesterday}`);
  return yesterday;
}

function InitialiseCharts() {
  InitialiseSolarChart();
  InitialiseBatteryChart();
  InitialiseWeatherChart();
}

function GetStats()
{
  //console.log("url : " + localURL + range);
  // console.log( "using " + useURL );

  startDate = OneDayAgo()
  startDate = new Date(startDate.toISOString());
  // console.log(`startDate ${startDate} `);

  $.ajax({ 
    url: useURL, // url: useURL + range,
    timeout: 30000,
    cache: false,
    crossDomain: true,
    type: "GET",
    dataType: 'text'
  })
  .done(function(data) {
    // console.log(data);
    var dataLines = data.split("\n");
    // console.log(`rows returned ${dataLines.length}`);
    // console.log(`first row ${dataLines[1]}`);
    // console.log(`last  row ${dataLines[dataLines.length-2]}`);

    AddAllData( dataLines );  // add data to keyed array for use by panel and display
    UpdatePanelValues( allData );
    RenderSolarChart( allData );
    RenderBatteryChart( allData );
    RenderWeatherChart( allData );
    UpdateSolarSummary( allData );
  })
  .fail( function(qXHR, textStatus, errorThrown) {
    console.log("failed with: " + errorThrown);
  });
}

function AddAllData( dataLines ) {

  allData = []; // zero out all previous data
  var dataValues = {};

  for ( var i = 0; i < dataLines.length; i++ ) {

    if (dataLines[i].length < 20 ) continue;

    dataLines[i] = dataLines[i].replace(/,\s*$/, ""); // remove trailing commas and spaces

    var dataItems = dataLines[i].split(",");
    // console.log(dataItems);

    // ignore items outside the time range of 24 hours
    dataValues.dateTime = dataItems[ 0 ];
    var dataDate = new Date(dataValues.dateTime);
    if ( dataDate.getTime() < startDate.getTime() ) {
      // console.log(`dataDate ${dataDate}, startDate ${startDate}`);
      continue;
    }

    dataValues.messageType = dataItems[ 1 ];

    if ( dataItems.length != 20 ) continue;

    for ( var j = 2; j < dataItems.length; j++ ) {
      var namePair = dataItems[j].split("=");
      // console.log(`name,value ${namePair[0]},${namePair[1]}`)
      dataValues[namePair[0]] = namePair[1];
    }

    if ( InvalidData( dataValues ) ) continue;

    var rowData = { 
      x: dataValues.dateTime,
      "temperature": parseFloat( dataValues.temp),
      "humidity": parseFloat( dataValues.humid),
      "batteryVoltage": parseFloat( dataValues.V) / 1000.0,
      "batteryCurrent": parseFloat(dataValues.I),
      "batteryStatus": dataValues.CS,
      "solarVoltage": parseFloat(dataValues.VPV) / 1000.0,
      "solarPower": dataValues.PPV,
      "solarStatus": parseInt(dataValues.MPPT),
      "loadStatus": dataValues.LOAD ,
      "loadCurrent": dataValues.IL
    };

    allData.push( rowData );
  }
    // console.log(`allData size after load ${allData.length}`);
    // console.log(allData[10]);
}

function InvalidData( dataValues) {

  if ( parseFloat( dataValues.V) < 5000 || parseFloat( dataValues.V) > 17000 ) return true;  // battery voltage

  return false;
}