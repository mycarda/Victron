const batteryVoltageColourOff        = '#777';
const batteryVoltageColourBulk       = '#5AF';
const batteryVoltageColourAbsorption = '#3BF';
const batteryVoltageColourFloat      = '#00F';
const batteryVoltageColourEqualise   = '#F77';
const batteryCurrentColour = '#FF1111'

var batteryChart;
var batteryData = { 
  datasets : [
    { 
      label : 'voltage (V)',
      pointRadius:0,
      // borderColor: batteryVoltageColour,
      segment: {
        borderColor: ((ctx) => {
          // console.log(ctx.p0.raw.batteryStatus);
          if ( ctx.p0.raw.batteryStatus == 0 ) {
              return batteryVoltageColourOff;
          }
          if ( ctx.p0.raw.batteryStatus == 3 ) {
              return batteryVoltageColourBulk;
          }
          if ( ctx.p0.raw.batteryStatus == 4 ) {
              return batteryVoltageColourAbsorption;
          }
          if ( ctx.p0.raw.batteryStatus == 5 ) {
              return batteryVoltageColourFloat;
          }
          if ( ctx.p0.raw.batteryStatus == 7 ) {
              return batteryVoltageColourEqualise;
          }
        })
      },
      backgroundColor: batteryVoltageColourFloat,
      borderWidth: 1,
      yAxisID: 'y',
      parsing: {
        yAxisKey: 'batteryVoltage'
      },
      data:[]
    },
    {
      label : 'current (mA)',
      pointRadius:0,
      borderColor: batteryCurrentColour,
      backgroundColor: batteryCurrentColour,
      borderWidth: 1,
      yAxisID: 'y1',
      parsing: {
        yAxisKey: 'batteryCurrent'
      },
      data:[]
    }
  ] 
};

function InitialiseBatteryChart() {
  var batteryCanvas = document.getElementById('batteryChart');
  batteryChart =  new Chart( batteryCanvas, {
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
          text: 'Battery',
          align: 'middle',
          font: { size: 24 }
        },
        // annotation: {
        //   annotations: {
        //     label1: {
        //       type: 'label',
        //       xValue: 30,
        //       yValue: 30,
        //       backgroundColor: '#777',
        //       content: ['This is my text', 'This is my text, second line'],
        //       font: {
        //         size: 18
        //       }
        //     }
        //   }
        // }
      },
      scales: {
        x: { type: 'time', time: { unit: 'hour', displayFormats: {hour: 'HH'} } },
        y: { max : 15.0, min : 11.0, ticks: { color: '#00F', beginAtZero:'#00F' }  },
        y1: { max : 4000, min : -4000, ticks: { color: '#F00', beginAtZero:'#F00' } }
      }
    }
  });
}

function RenderBatteryChart( data ) {

  batteryData.datasets[0].data = data;
  batteryData.datasets[1].data = data;
  batteryChart.data = batteryData;
  batteryChart.update();
}