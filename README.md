# TP 2021 1C-cualquier-cosa

## Estructura de Proyecto
Estructura basada en el [aporte de RaniAgus](https://github.com/RaniAgus/utnso-project) con guia inclusive sobre sus features.
Adicionalmente hizo una [breve explicacion](https://github.com/RaniAgus/utnso-project/wiki/Eclipse) sobre como importarlo a tu ide preferido, en este caso utilizamos eclipse 

IMPORTANTE: Ante cualquier duda sobre como utilizar esta estructura consultar esos repositorios.

## En este branch dejo las utils verificadas
Acá dejo un ejemplo que contiene usos correctos de las funciones definidas en las utils.
Ahora hay cuatro utils.
Utils-client, que servirá para cualquier dispositivo que necesite las funcionalidades de un cliente TCP.
Utils-server, que realiza todas las funciones que necesita un servidor para establecer conexión con un cliente.
Utils-sockets, que tiene utilidades comunes en el trato de sockets, como obtener un puerto a partir de un socket y validar una conexión.
Utils-mensajes, que comprende todo el manejo de la comunicación entre distintos sockets y define todos los mensajes.

### Observaciones:
En la prueba del ejemplo no me funcionó cuando alterné el orden de mensajes. Es un bug raro, ya que en las pruebas de rama-ram donde se usan esas funciones no hubo problemas. A estar atentos.
Si el servidor se detiene abruptamente, el puerto no se libera y habrá que cambiarlo para poder hacer una nueva prueba o activar el servidor nuevamente.
Dado el caso en que el servidor se detiene abruptamente, puede que el cliente, al intentar conectarse al puerto donde estaba funcionando el servidor, piense que lo logre. Al menos sucedió en una prueba que, como no actualicé el puerto en el config, se dio a relucir el hecho de que el cliente se "conectó" y siguió ejecutando el código. Esto puede que no sea un problema, ya que al intentar comunicarse por mensajes el cliente fallará y podrá realizar el procedimiento correspondiente para cuando se rompe la conexión.
