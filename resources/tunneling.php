<?php

set_time_limit(0);
ob_implicit_flush();

$address = '0.0.0.0';
$port = 8080;

$sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_set_option($sock, SOL_SOCKET, SO_REUSEADDR, 1);
socket_bind($sock, $address, $port);
socket_listen($sock);

$clients = [];

function handshake($client) {
    $handshake = socket_read($client, 1024);
    preg_match('#Sec-WebSocket-Key: (.*?)\r\n#U', $handshake, $matches);
    $key = base64_encode(pack('H*', sha1($matches[1] . '258EAFA5-E914-47DA-95CA-C5AB0DC85B11')));
    $response = "HTTP/1.1 101 Switching Protocols\r\n"
        . "Upgrade: websocket\r\n"
        . "Connection: Upgrade\r\n"
        . "Sec-WebSocket-Accept: " . $key . "\r\n\r\n";
    socket_write($client, $response, strlen($response));
    return true;
}

while (true) {
    $read = array($sock);
    foreach ($clients as $client) {
        $read[] = $client;
    }

    socket_select($read, $write = NULL, $except = NULL, NULL);

    if (in_array($sock, $read)) {
        $client = socket_accept($sock);
        if ($client !== false) {
            if (handshake($client)) {
                $clients[] = $client;
                echo "New client connected and handshake completed\n";
            } else {
                socket_close($client);
                echo "Handshake failed. Client disconnected.\n";
            }
        }
    }

    foreach ($clients as $key => $client) {
        if (in_array($client, $read)) {
            $input = @socket_read($client, 1024);
            if ($input === false || $input === "") {
                socket_close($client);
                unset($clients[$key]);
                echo "Client disconnected\n";
            } else {
                $message = trim($input);
                echo "Received: " . $message . "\n";

                foreach ($clients as $otherClient) {
                    if ($otherClient != $client) {
                        socket_write($otherClient, $message . "\n"); // Keep newline for separation
                    }
                }
            }
        }
    }
}

socket_close($sock);

?>