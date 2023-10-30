<?php
    /* Conectando, escolhendo o banco de dados */
    $link = mysqli_connect("localhost", "root", "")or die("Nao pude conectar: " . mysqli_error());
    mysqli_select_db($link,"Reservoir") or die("N„o pude selecionar o banco de dados");

    // Update the database based on the status
    $sql = "SELECT * FROM Trab4 WHERE Id = 1"; // Adjust the query based on your table structure

    // Get query result
    $result = mysqli_query($link,$sql) or die("A query falhou: " . mysqli_error());
    $linha1=mysqli_fetch_array($result,MYSQLI_ASSOC);
   
    //send as json
    echo json_encode($linha1);

    // Close the connection
    $link->close();
?>