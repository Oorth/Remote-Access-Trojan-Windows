<?php
if (isset($_FILES["file"]))
{
    $target_dir = "uploads/"; // Directory where you want to save the files
    $target_file = $target_dir . basename($_FILES["file"]["name"]);
    if (move_uploaded_file($_FILES["file"]["tmp_name"], $target_file)) {
        echo "File uploaded successfully.";
    } else {
        echo "Error uploading file.";
    }
} else {
    echo "No file received.";
}
?>