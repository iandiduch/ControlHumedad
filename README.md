# ControlHumedad
1. En la Carpeta ESP32 se encuentra todo el desarrollo que luego se quemo dentro del ESP32 para conectarse con los
sensores y luego enviar los POST al servidor con Flask en la Raspberry PI

2. En la carpeta Raspberry se encuentra el .py donde se desarrollo el servidor con Flask, se podran encontrar
los metodos Get y Post definidos, asi como tambien se desarrollo el guardado de las lecturas en una base de datos.
Ademas se incorporo con AWS IoT el protocolo mqtt el cual a medida que los datos se leen por el POST se va haciendo
una publicacion para todo aquel que este escuchando por mqtt.
