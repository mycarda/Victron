const humidityColour      = '#00F';
const temperatureColour   = '#FF1111'

var weatherChart;
var weatherData = { 
  datasets : [
    { 
      label : 'temperature (C)',
      pointRadius:0,
      borderColor: temperatureColour,
      backgroundColor: temperatureColour,
      borderWidth: 1,
      yAxisID: 'y',
      parsing: {
        yAxisKey: 'temperature'
      },
      data:[]
    },
    {
      label : 'relative humidity (%)',
      pointRadius:0,
      borderColor: humidityColour,
      backgroundColor: humidityColour,
      borderWidth: 1,
      yAxisID: 'y1',
      parsing: {
        yAxisKey: 'humidity'
      },
      data:[]
    }
  ] 
};

function InitialiseWeatherChart() {
  var weatherCanvas = document.getElementById('weatherChart');
  weatherChart =  new Chart( weatherCanvas, {
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
          text: 'Weather',
          align: 'middle',
          font: { size: 24 }
        },
      },
      scales: {
        x: { type: 'time', time: { unit: 'hour', displayFormats: {hour: 'HH'} } },
        y: { max : 40.0, min : -10.0, ticks: { color: '#F00', beginAtZero:'#F00' }  },
        y1: { max : 100, min : 0, ticks: { color: '#00F', beginAtZero:'#00F' }  }
      }
    }
  });
}

function RenderWeatherChart( data ) {

  weatherData.datasets[0].data = data;
  weatherData.datasets[1].data = data;
  weatherChart.data = weatherData;
  weatherChart.update();
}