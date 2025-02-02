#include "RequestParser.hpp"
#include "gtest/gtest.h"
#include <stdexcept>

class MultipartFormdataTest : public ::testing::Test {
protected:
	MultipartFormdataTest()
		: request()
	{
		request.boundary = "WebKitFormBoundary7MA4YWxkTrZu0gW";
		request.targetResource = "/workspaces/webserv/html/uploads/";
	}
	~MultipartFormdataTest() override { }

	RequestParser p;
	HTTPRequest request;
};

TEST_F(MultipartFormdataTest, ParseHeader)
{
	// Arrange
	request.boundary = "";
	const std::string headerString = "POST /upload HTTP/1.1\r\nHost: example.com\r\nContent-Type: multipart/form-data; "
									 "boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Length: 195";
	// Act
	p.parseHeader(headerString, request);

	// Assert
	EXPECT_TRUE(request.hasMultipartFormdata);
	EXPECT_EQ(request.boundary, "----WebKitFormBoundary7MA4YWxkTrZu0gW");
}

TEST_F(MultipartFormdataTest, ParseHeaderNoBoundary)
{
	// Arrange
	request.boundary = "";
	const std::string headerString
		= "POST /upload HTTP/1.1\r\nHost: example.com\r\nContent-Type: multipart/form-data\r\n"
		  "Content-Length: 195";

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.parseHeader(headerString, request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_BAD_MULTIPART_FORMDATA, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(MultipartFormdataTest, DecodeBody)
{
	// Arrange
	request.body
		= "--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"username"
		  "\"\r\n\r\nBatman\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; "
		  "name=\"file\"; "
		  "filename=\"darkknight.txt\"\r\nContent-Type: text/plain\r\n\r\nSome men just want to watch the world "
		  "burn.\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";
	// Act
	p.decodeMultipartFormdata(request);

	// Assert
	EXPECT_EQ(request.targetResource, "/workspaces/webserv/html/uploads/darkknight.txt");
	EXPECT_EQ(request.body, "Some men just want to watch the world burn.");
}

TEST_F(MultipartFormdataTest, DecodeBodyNoStartBoundary)
{
	// Arrange
	request.body
		= "Content-Disposition: form-data; "
		  "name=\"file\"; "
		  "filename=\"darkknight.txt\"\r\nContent-Type: text/plain\r\n\r\nSome men just want to watch the world "
		  "burn.\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.decodeMultipartFormdata(request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_BAD_MULTIPART_FORMDATA, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(MultipartFormdataTest, DecodeBodyNoContentDisposition)
{
	// Arrange
	request.body
		= "--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"username"
		  "\"\r\n\r\nBatman\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
          "name=\"file\"; "
		  "filename=\"darkknight.txt\"\r\nContent-Type: text/plain\r\n\r\nSome men just want to watch the world "
		  "burn.\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.decodeMultipartFormdata(request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_BAD_MULTIPART_FORMDATA, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(MultipartFormdataTest, DecodeBodyNoFilename)
{
	// Arrange
	request.body = "--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"username"
				   "\"\r\n\r\nBatman\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; "
				   "name=\"file\";"
				   "\r\nContent-Type: text/plain\r\n\r\nSome men just want to watch the world "
				   "burn.\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.decodeMultipartFormdata(request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_BAD_MULTIPART_FORMDATA, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(MultipartFormdataTest, DecodeBodyNoContentType)
{
	// Arrange
	request.body = "--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"username"
				   "\"\r\n\r\nBatman\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; "
				   "name=\"file\"; "
				   "filename=\"darkknight.txt\"\r\n\r\nSome men just want to watch the world "
				   "burn.\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.decodeMultipartFormdata(request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_BAD_MULTIPART_FORMDATA, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(MultipartFormdataTest, DecodeBodyNoCRLFCRLF)
{
	// Arrange
	request.body = "--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"username"
				   "\"\r\n\r\nBatman\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; "
				   "name=\"file\"; "
				   "filename=\"darkknight.txt\"\r\nContent-Type: text/plain\r\nSome men just want to watch the world "
				   "burn.\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.decodeMultipartFormdata(request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_BAD_MULTIPART_FORMDATA, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(MultipartFormdataTest, DecodeBodyNoEndBoundary)
{
	// Arrange
	request.body
		= "--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"username"
		  "\"\r\n\r\nBatman\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; "
		  "name=\"file\"; "
		  "filename=\"darkknight.txt\"\r\nContent-Type: text/plain\r\n\r\nSome men just want to watch the world "
		  "burn.\r\nbliblablubWebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.decodeMultipartFormdata(request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_BAD_MULTIPART_FORMDATA, e.what());
				throw;
			}
		},
		std::runtime_error);
}

TEST_F(MultipartFormdataTest, DecodeBodyMultipleUploads)
{
	// Arrange
	request.body
		= "--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"username"
		  "\"\r\n\r\nBatman\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; "
		  "name=\"file\"; "
		  "filename=\"darkknight.txt\"\r\nContent-Type: text/plain\r\n\r\nSome men just want to watch the world "
		  "burn.\r\n"
		  "--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"username"
		  "\"\r\n\r\nJoker\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; "
		  "name=\"file\"; "
		  "filename=\"?.txt\"\r\nContent-Type: text/plain\r\n\r\nWant me to show you a magic trick? "
		  "\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

	// Act & Assert
	EXPECT_THROW(
		{
			try {
				p.decodeMultipartFormdata(request);
			} catch (const std::runtime_error& e) {
				EXPECT_STREQ(ERR_MULTIPLE_UPLOADS, e.what());
				throw;
			}
		},
		std::runtime_error);
}
