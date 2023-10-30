<?php
    /* Conectando, escolhendo o banco de dados */
    $link = mysqli_connect("localhost", "root", "")or die("Nao pude conectar: " . mysqli_error());
    mysqli_select_db($link,"Reservoir") or die("N„o pude selecionar o banco de dados");

    // Get the status from the query parameters
    $status = (int) $_GET['status'];

    // Update the database based on the status
    $sql = "UPDATE cnt SET led_control = '$status' WHERE id = 1"; // Adjust the query based on your table structure

    // Execute the query
    if ($link->query($sql) === TRUE) {
        echo "Record updated successfully";
    } else {
        echo "Error updating record: " . $link->error;
    }

    // Close the connection
    $link->close();
?>