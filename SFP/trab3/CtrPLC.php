<html>
  <body>
    ------- Controlo -------
    <form action="CtrPLC.php" method="POST">
      Introduza os valores de Y0, Y1, e Y2 <br>Estes valores serao escritos na base de dados "Reservatorio,"<br>
      na tabela "SupervisaoReservatorio(Leitura,Data,Hora,Y0,Y1,Y2,X0,X1,X2,X3)"<br><br>
      Y0:<input type="text" name="saidaY0" value="1">
      Y1:<input type="text" name="saidaY1" value="1">
      Y2:<input type="text" name="saidaY2" value="1">
      X0:<input type="text" name="leituraX0" value="1">
      X1:<input type="text" name="leituraX1" value="1">
      X2:<input type="text" name="leituraX2" value="1">
      X3:<input type="text" name="leituraX3" value="1">
      <input type="submit" value="enviar">
    </form>

    <?php
      $Y0=$_POST['saidaY0'];
      $Y1=$_POST['saidaY1'];
      $Y2=$_POST['saidaY2'];
      $X0=$_POST['leituraX0'];
      $X1=$_POST['leituraX1'];
      $X2=$_POST['leituraX2'];
      $X3=$_POST['leituraX3'];

      /* Conectando, escolhendo o banco de dados */
      $link = mysqli_connect("localhost", "root", "")or die("Nao pude conectar: " . mysqli_error());
      mysqli_select_db($link,"Reservoir") or die("Noa pude selecionar o banco de dados");
      
      /* Query SQL de actualizacao */
      $query = "INSERT INTO Supervision (Id, Y0, Y1, Y2, X0, X1, X2, X3)
            VALUES (1, '$Y0', '$Y1', '$Y2', '$X0', '$X1', '$X2', '$X3')
            ON DUPLICATE KEY UPDATE
                Y0 = VALUES(Y0),
                Y1 = VALUES(Y1),
                Y2 = VALUES(Y2),
                X0 = VALUES(X0),
                X1 = VALUES(X1),
                X2 = VALUES(X2),
                X3 = VALUES(X3);" ;
      
      $result = mysqli_query($link,$query) or die("A query falhou: " . mysqli_error());
      echo "------- Supervisao -------<br>Os valores lidos na base de dados sao:";
      
      /* Fazendo a query SQL DE LEITURA DA BASE DE DADOS*/
      $query = "SELECT * FROM Supervision";
      $result = mysqli_query($link,$query) or die("A query falhou: " . mysqli_error());
      $linha1=mysqli_fetch_array($result,MYSQLI_ASSOC);
      echo " Y0:".$linha1['Y0'];
      echo " Y1:".$linha1['Y1'];
      echo " Y2:".$linha1['Y2'];
      echo " X0:".$linha1['X0'];
      echo " X1:".$linha1['X1'];
      echo " X2:".$linha1['X2'];
      echo " X3:".$linha1['X3'];
      
      /* Liberta o resultado */
      mysqli_free_result($result);
      /* Fechando a conex„o */
      mysqli_close($link);
    ?>
  </body>
</html>
