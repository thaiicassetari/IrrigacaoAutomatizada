import serial
import mysql.connector

comport = serial.Serial('COM3', 9600)
print('Serial Iniciada...\n')

cnx = mysql.connector.connect(
  user='arduino',
  password='arduino123',
  host='localhost',
  database='arduino_teste'
)

cursor = cnx.cursor()
add_sinais = ("INSERT INTO sinais "
              "(sin_umidade, sin_hora, sin_resultado, "
              "sin_acao, sin_ano, sin_mes, sin_dia) "
              "VALUES "
              "(%s, %s,%s, %s,%s, %s,%s)")

while True:
  serialValue = str(comport.readline())
  characters = "b'"
  for x in range(len(characters)):
    serialValue = serialValue.replace(characters[x], "")

  data_sinais = [x.strip() for x in serialValue.split("|")][:-1]
  print(data_sinais)
  cursor.execute(add_sinais, data_sinais)
  cnx.commit()

cnx.close()
