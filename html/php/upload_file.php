<?php
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
	// Define the directory to store the uploaded files
	$target_dir = "../uploads/";
	// Specify the target file path
	$target_file = $target_dir . basename($_FILES["file"]["name"]);
	$uploadOk = 1;

	// Check if the file was uploaded without errors
	if (isset($_FILES["file"])) {
		// Check for file upload errors
		if ($_FILES["file"]["error"] == 0) {
			// Move the file to the uploads directory
			if (move_uploaded_file($_FILES["file"]["tmp_name"], $target_file)) {
				echo "The file ". htmlspecialchars(basename($_FILES["file"]["name"])). " has been uploaded.";
			} else {
				echo "Sorry, there was an error uploading your file.";
			}
		} else {
			echo "Error: " . $_FILES["file"]["error"];
		}
	} else {
		echo "No file uploaded.";
	}
}
?>
