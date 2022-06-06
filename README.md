# -Graduate-work-Bachelor-
Application of the local voting protocol for state synchronization in a decentralized network of mobile nodes

## Introduction
Modern realities create new problems and tasks that can be effectively solved with the help of groups of robots.

For the organization of group interaction, two main directions can be distinguished: centralized and decentralized. With centralized interaction, there is a control and information processing center that communicates with all nodes in the group. With decentralized - nodes communicate with each other, each node manages itself. Also, information processing occurs at each node.

To create a well-functioning decentralized group on each node, it is necessary to have an up-to-date view of the state of the entire system.

For the coordinated operation of all nodes, the representation on each node, ideally, should be the same - the task of synchronizing the representations of the state of the system on all nodes appears.

## Method
The state synchronization method in a decentralized network of mobile nodes includes a synchronization method - a local voting protocol and a data transmission method.

The local voting protocol is responsible for the synchronization on each node. It receives views from other nodes, generates a view of the node about the state of the system based on the received views and data from the aggregates and sensors of the node. Next, the generated view is sent using the data transfer method.

To create the most efficient method of synchronization, you need to abandon the routing when passing messages. And the representation of the state of the system from each node should be broadcast to all neighbors.

<img src="[https://electronov.net/wp-content/uploads/2017/09/esp32-board-front-600.jpg](https://www.overleaf.com/project/628cec6c0db8569546473157/file/629088c1e61cdbe8adb78cea)" width="30%">

### Recommended board to use - ESP32
<img src="https://electronov.net/wp-content/uploads/2017/09/esp32-board-front-600.jpg" width="30%">
