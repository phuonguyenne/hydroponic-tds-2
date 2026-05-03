<?php
if(!file_exists("mode.txt")){
    file_put_contents("mode.txt","non");
}
echo file_get_contents("mode.txt");
?>