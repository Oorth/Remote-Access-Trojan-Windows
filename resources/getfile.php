<?php
if (isset($_FILES["file"])) {
    $target_dir = "uploads/"; // Directory where you want to save the files

    // Check if the directory exists, if not, create it
    if (!is_dir($target_dir)) {
        mkdir($target_dir, 0755, true);
    }

    $target_file = $target_dir . basename($_FILES["file"]["name"]);

    // Validate file type (example: allow only images)
    $file_type = strtolower(pathinfo($target_file, PATHINFO_EXTENSION));
    $allowed_types = ['jpg', 'jpeg', 'png', 'gif']; // Add more allowed types as needed

    if (!in_array($file_type, $allowed_types)) {
        echo "Error: Only JPG, JPEG, PNG & GIF files are allowed.";
        exit;
    }

    // Sanitize file name
    $target_file = $target_dir . uniqid() . '.' . $file_type;

    if (move_uploaded_file($_FILES["file"]["tmp_name"], $target_file)) {
        echo "File uploaded successfully.";
    } else {
        echo "Error uploading file.";
    }
} else {
    echo "No file received.";
}
?>