# Application of the local voting protocol for state synchronization in a decentralized network of mobile nodes

## Introduction
Modern realities create new problems and tasks that can be effectively solved with the help of groups of robots.

For the organization of group interaction, two main directions can be distinguished: centralized and decentralized. With centralized interaction, there is a control and information processing center that communicates with all nodes in the group. With decentralized - nodes communicate with each other, each node manages itself. Also, information processing occurs at each node.

To create a well-functioning decentralized group on each node, it is necessary to have an up-to-date view of the state of the entire system.

For the coordinated operation of all nodes, the representation on each node, ideally, should be the same - the task of synchronizing the representations of the state of the system on all nodes appears.

## Method
The state synchronization method in a decentralized network of mobile nodes includes a synchronization method - a local voting protocol and a data transmission method.

The local voting protocol is responsible for the synchronization on each node. It receives views from other nodes, generates a view of the node about the state of the system based on the received views and data from the aggregates and sensors of the node. Next, the generated view is sent using the data transfer method.

To create the most efficient method of synchronization, you need to abandon the routing when passing messages. And the representation of the state of the system from each node should be broadcast to all neighbors.

## Implementation
To implement the exchange of views using WiFi technology, the Arduino library of the same name is used.

The following message format is chosen to transmit the presentation - the payload is immediately after the IEEE 802.11 MAC header. Next, the library forms a WiFi frame and passes it further up the stack. Information is received by viewing WiFi frames without connecting to a WiFi point (sniffer), that is, there is no time loss for this connection. Next, useful information is extracted from the captured frames.

The WiFi package, with which the developed method works, is formed by the WiFi library and includes service information - about the signal strength and the WiFi channel on which the transmission was made, as well as the WiFi frame. The WiFi frame, in turn, contains service information - a MAC header, payload information, and a checksum.

Thanks to this approach, one message exchange-receive cycle takes less than 100 milliseconds.

### Recommended board to use - ESP32
https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf

<img src="https://electronov.net/wp-content/uploads/2017/09/esp32-board-front-600.jpg" width="30%">

### Recommended source code editor - VS Code with PlatformIO IDE extension
- https://code.visualstudio.com/
- https://platformio.org/platformio-ide
