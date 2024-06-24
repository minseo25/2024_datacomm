# A stream-based messaging service

• During connection, the client and the server repeat the following 
operations several times until the user presses ‘bye\n’

• When the user presses ‘bye\n’, the client sends ‘Echo_CLOSE’ to the 
server.

• When receiving ‘Echo_CLOSE’, the server replies with `Echo_CLOSE’ 
and closes the connection with the corresponding client.

• The client that has received ‘Echo_CLOSE’ closes the connection.

### [Client]
– The Client process stores the messages from the user in queue, until the user 
presses ‘Q\n’.

– When receiving ‘Q\n’, the client sends ‘SEND’, the queued messages, and ‘RECV’

### [Server]
– When the sever receives ‘SEND’, it queues the messages until receiving ‘RECV’. 

– When receiving ‘RECV’, the server process sends all received messages orderly to 
the client


![image](https://github.com/minseo25/PA1/assets/52146976/9fae4631-bb2d-48c8-8f09-6bddb7c016c8)
