import paho.mqtt.client as mqtt
import sys
import mysql.connector
from mysql.connector import Error
from datetime import datetime
import pytz

# Define o fuso horário do Brasil
fuso_horario_brasil = pytz.timezone('America/Sao_Paulo')

conexaosql = mysql.connector.connect(user='root', password='R00tr00t*',
                                     host='54.215.193.95', database='db_pet')


def on_connect(client, userdata, flags, rc):
    print("Conectado - Codigo de Resultado " + str(rc))

    client.subscribe('reservatorio-1/volume')


def on_message(client, obj, msg):
    print(msg.topic + " " + str(msg.payload))
    mensagem = str(msg.payload.decode("utf-8"))
    # Obtém a data e hora atual do Brasil
    data_atual_brasil = datetime.now(fuso_horario_brasil)
    # Formata a data e hora no formato desejado
    data_formatada = data_atual_brasil.strftime('%d/%m/%Y %H:%M')
    cursor = conexaosql.cursor()
    cursor.execute("""INSERT INTO dados (data, volume)
                              VALUES ('%s', %s)""" % (str(data_formatada), mensagem))

    conexaosql.commit()
    cursor.close()


client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
# Assign event callbacks
client.on_message = on_message
client.on_connect = on_connect

try:
    client.connect('54.215.193.95', 1883, 60)
except:
    print('Conexão não foi estabelecida com o MQTT')
    print('Encerrando')
    sys.exit()

client.username_pw_set('', '')

try:
    client.loop_forever()
except KeyboardInterrupt:  # ctrl + c
    print('Encerrando')