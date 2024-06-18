document.addEventListener('DOMContentLoaded', () => {
    const canvas = document.getElementById('humidityCanvas');
    const ctx = canvas.getContext('2d');
    const showGraphBtn = document.getElementById('showGraphBtn');
    const lineChartCanvas = document.getElementById('lineChartCanvas');
    const lineChartCtx = lineChartCanvas.getContext('2d');

    let backgroundImage = new Image();
    backgroundImage.src = 'images/background.png';
    backgroundImage.onload = function() {
        fetchData(); // Fetch data once the image is loaded
    };

    function fetchData() {
        fetch(window.apiUrl + '/last')
            .then(response => {
                if (!response.ok) {
                    throw new Error('Network response was not ok ' + response.statusText);
                }
                return response.json();
            })
            .then(data => {
                if (data.humidity1 !== undefined && data.humidity2 !== undefined && data.humidity3 !== undefined) {
                    drawData(data); // Draw the data from the latest sensor record
                } else {
                    console.error('No valid data received');
                }
            })
            .catch(error => {
                console.error('There was a problem with the fetch operation:', error);
            });
    }

    function drawData(data) {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        
        // Draw the background image
        ctx.drawImage(backgroundImage, 0, 0, canvas.width, canvas.height);

        const rectWidth = 200;
        const rectHeight = 70;

        const rectangles = [
            { x: 750, y: 150, value: data.humidity1 },  // Posición y valor del primer rectángulo
            { x: 50, y: 570, value: data.humidity2 }, // Posición y valor del segundo rectángulo
            { x: 50, y: 830, value: data.humidity3 }  // Posición y valor del tercer rectángulo
        ];

        rectangles.forEach(rect => {
            ctx.fillStyle = 'rgba(0, 0, 0, 0.5)';
            ctx.fillRect(rect.x, rect.y, rectWidth, rectHeight);

            ctx.fillStyle = 'white';
            ctx.font = '30px Arial';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(`${rect.value}%`, rect.x + rectWidth / 2, rect.y + rectHeight / 2);
        });
    }

    function fetchAllData() {
        fetch(window.apiUrl + '/all')
            .then(response => {
                if (!response.ok) {
                    throw new Error('Network response was not ok ' + response.statusText);
                }
                return response.json();
            })
            .then(data => {
                if (data.length > 0) {
                    drawLineChart(data); // Draw the line chart with all sensor data
                } else {
                    console.error('No data received');
                }
            })
            .catch(error => {
                console.error('There was a problem with the fetch operation:', error);
            });
    }

    function drawLineChart(data) {
        // Extract timestamps and humidity values
        const labels = data.map(entry => entry.timestamp);
        const humidity1Values = data.map(entry => entry.humidity1);
        const humidity2Values = data.map(entry => entry.humidity2);
        const humidity3Values = data.map(entry => entry.humidity3);

        const chartData = {
            labels: labels,
            datasets: [
                {
                    label: 'Humedad 1',
                    borderColor: 'rgba(75, 192, 192, 1)',
                    backgroundColor: 'rgba(75, 192, 192, 0.2)',
                    fill: false,
                    data: humidity1Values,
                },
                {
                    label: 'Humedad 2',
                    borderColor: 'rgba(54, 162, 235, 1)',
                    backgroundColor: 'rgba(54, 162, 235, 0.2)',
                    fill: false,
                    data: humidity2Values,
                },
                {
                    label: 'Humedad 3',
                    borderColor: 'rgba(255, 206, 86, 1)',
                    backgroundColor: 'rgba(255, 206, 86, 0.2)',
                    fill: false,
                    data: humidity3Values,
                }
            ]
        };

        // Create line chart
        new Chart(lineChartCtx, {
            type: 'line',
            data: chartData,
            options: {
                responsive: true,
                title: {
                    display: true,
                    text: 'Variaciones de Humedad en el Tiempo'
                },
                scales: {
                    xAxes: [{
                        type: 'time',
                        time: {
                            unit: 'minute',
                            tooltipFormat: 'll HH:mm'
                        },
                        scaleLabel: {
                            display: true,
                            labelString: 'Tiempo'
                        }
                    }],
                    yAxes: [{
                        scaleLabel: {
                            display: true,
                            labelString: 'Humedad (%)'
                        }
                    }]
                }
            }
        });
    }

    showGraphBtn.addEventListener('click', () => {
        lineChartCanvas.style.display = 'block';
        fetchAllData();
    });

    setInterval(fetchData, 5000); // Fetch data every 5 seconds
});

