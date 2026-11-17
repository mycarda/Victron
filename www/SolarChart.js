const solarVoltageColourOff     = '#aaa'
const solarVoltageColourTracker = '#3af'
const solarVoltageColourLimited = '#00F'
const solarPowerColour = '#11AA11'

var solarChart;
var solarData = { 
  datasets : [
    { 
      label : 'voltage (V)',
      pointRadius:0,
      // borderColor: solarVoltageColour,
      backgroundColor: solarVoltageColourLimited,
      segment: {
        borderColor: ((ctx) => {
          if ( ctx.p0.raw.solarStatus == 2 ) { // tracker mode
            return solarVoltageColourTracker;
          }
          if ( ctx.p0.raw.solarStatus == 1 ) { // voltage or current limited
            return solarVoltageColourLimited;
          }
          return solarVoltageColourOff;  // off
        })
      },
      borderWidth: 1,
      yAxisID: 'y',
      parsing: {
        yAxisKey: 'solarVoltage'
      },
      data:[]
    },
    {
      label : 'power (W)',
      pointRadius:0,
      borderColor: solarPowerColour,
      backgroundColor: solarPowerColour,
      borderWidth: 1,
      yAxisID: 'y1',
      parsing: {
        yAxisKey: 'solarPower'
      },
      data:[]
    },
  ] 
};

function InitialiseSolarChart() {
  var solarCanvas = document.getElementById('solarChart');
  solarChart =  new Chart( solarCanvas, {
    type: 'line',
    normalized: true,
    responsive: true,
    data: {},
    options: {
      animation: false,
      fill: false,
      maintainAspectRatio: false,
      plugins: {
        title: {
          display: true,
          text: 'solar',
          align: 'middle',
          font: { size: 24 }
        }
      },
      scales: {
        x: { type: 'time', time: { unit: 'hour', displayFormats: {hour: 'HH'} } },
        y: { max : 40.0, min : 0.0, ticks: { color: '#00F', beginAtZero:'#00F' }  },
        y1: { max : 80, min : 0, ticks: { color: '#0A0', beginAtZero:'#0A0' }  }
      }
    }
  });
}

function RenderSolarChart( data ) {

  solarData.datasets[0].data = data;
  solarData.datasets[1].data = data;
  solarChart.data = solarData;
  solarChart.update();
}