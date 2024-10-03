# robot-arm

Hosting Web Socket Server:

- Open Command Prompt (or equivalent).
- Navigate to the "web-sockets-server" folder.
- Run "npm i" to install packages.
- Run "node server.js" to run server.

Hosting Website:

- Open Command Prompt (or equivalent).
- Navigate to the "client-portal" folder.
- Run "npm i" to install packages.
- Run "npm run dev" to run the web server.

Connecting to ESP-32S:

- Edit "Wifi Credientials in esp32s-arm.ino for whatever network you want.
- Connect to that same network as a private network.
- Host the Web Socket Server on that Network.
- The light on the ESP-32S will turn blue if connected to the Web Socket Server.
