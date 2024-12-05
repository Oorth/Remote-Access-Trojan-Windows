<?php
// Check if the file has been uploaded via POST request
if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_FILES['file'])) {
    // Retrieve the uploaded file data from the $_FILES array
    $file = $_FILES['file'];
    
    // Define the upload directory where the file will be stored
    $uploadDirectory = 'uploads/';
    
    // Get the file name and generate a safe file path
    $fileName = basename($file['name']);
    $destination = $uploadDirectory . $fileName;

    // Check if the file was uploaded successfully
    if ($file['error'] == UPLOAD_ERR_OK) {
        // Check if the file is not too large (e.g., limit to 50MB)
        if ($file['size'] > 50 * 1024 * 1024) {
            echo "Error: File is too large. Maximum allowed size is 50MB.";
            exit;
        }
        
        // Check the file type (e.g., only allow text files and images)
        $allowedMimeTypes = ['text/plain', 'image/jpeg', 'image/png'];
        if (!in_array($file['type'], $allowedMimeTypes)) {
            echo "Error: Invalid file type. Only text files, JPG, and PNG images are allowed.";
            exit;
        }

        // Move the uploaded file to the specified directory
        if (move_uploaded_file($file['tmp_name'], $destination)) {
            echo "File uploaded successfully to $destination";
        } else {
            echo "Error: Failed to upload file.";
        }
    } else {
        // Handle upload errors
        echo "Error: " . $file['error'];
    }
} else {
    echo "Error: No file uploaded.";
}
?>
