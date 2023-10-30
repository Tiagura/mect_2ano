<html>
    <body>
        ------- Controlo -------
        <form action="status.php" method="POST">
            Introduza os valores de Y0, Y1, e Y2 <br>Estes valores serao escritos na base de dados "Reservatorio,"<br>
            na tabela "SupervisaoReservatorio(X0,X1,X2,X3)"<br><br>
            X0:<input type="text" name="leituraX0" value="1" >
            X1:<input type="text" name="leituraX1" value="1" >
            X2:<input type="text" name="leituraX2" value="1" >
            X3:<input type="text" name="leituraX3" value="1" >
            <input type="submit" value="enviar">
        </form>

        <?php
            $X0=(int) $_POST['leituraX0'];
            $X1=(int) $_POST['leituraX1'];
            $X2=(int) $_POST['leituraX2'];
            $X3=(int) $_POST['leituraX3'];
            //unsigned int X3
            /* Conectando, escolhendo o banco de dados */
            $link = mysqli_connect("localhost", "root", "")or die("Nao pude conectar: " . mysqli_error());
            mysqli_select_db($link,"Reservoir") or die("N„o pude selecionar o banco de dados");
            /* Query SQL de actualizacao */
            $query = "INSERT INTO Trab4 (id, X0, X1, X2, X3)
            VALUES (1, '$X0', '$X1', '$X2', '$X3')
            ON DUPLICATE KEY UPDATE
                X0 = VALUES(X0),
                X1 = VALUES(X1),
                X2 = VALUES(X2),
                X3 = VALUES(X3);" ;
            $result = mysqli_query($link,$query) or die("A query falhou: " . mysqli_error());

            /* Query SQL de actualizacao valor do sensor fotovoltaico */
            $query = "INSERT INTO foto_values (X3)
            VALUES ('$X3');";
            $result = mysqli_query($link,$query) or die("A query falhou: " . mysqli_error());

            /* Faz query para ler o valor de led_control */
            $query = "SELECT led_control FROM cnt WHERE Id=1";
            $result = mysqli_query($link,$query) or die("A query falhou: " . mysqli_error());
            $linha1=mysqli_fetch_array($result,MYSQLI_ASSOC);
            echo " led_control:".$linha1['led_control'];

            /* Liberta o resultado */
            mysqli_free_result($result);

            /* Fechando a conex„o */
            mysqli_close($link);
        ?>
    </body>
</html>