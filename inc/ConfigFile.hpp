#pragma once

/* ====== LIBRARIES ====== */

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/* ====== DEFINITIONS ====== */

enum Method { MethodGet, MethodPost, MethodDelete, MethodCount };

struct Location {

public:
	Location(void);

	// Getter functions
	std::string getPath() const;
	std::string getRoot() const;
	std::vector<std::string> getIndices() const;
	std::string getCgiExt() const;
	std::string getCgiPath() const;
	bool getIsAutoindex() const;
	unsigned long getMaxBodySize() const;
	std::map<unsigned short, std::string> getErrorPages() const;
	bool getAllowedMethod(Method method) const;
	std::map<unsigned short, std::string> getReturns() const;

	// Setter functions
	void setPath(const std::string& path);
	void setRoot(const std::string& root);
	void setIndex(const std::string& index);
	void setCgiExt(const std::string& cgiExt);
	void setCgiPath(const std::string& cgiPath);
	void setIsAutoindex(bool isAutoindex);
	void setMaxBodySize(unsigned long maxBodySize);
	void setErrorPage(unsigned short code, const std::string& path);

private:
	std::string m_path;
	std::string m_root;
	std::vector<std::string> m_indices;
	std::string m_cgiExt;
	std::string m_cgiPath;
	bool m_isAutoindex;
	unsigned long m_maxBodySize;
	std::map<unsigned short, std::string> m_errorPage;
	bool m_allowedMethods[MethodCount];
	std::map<unsigned short, std::string> m_returns;
};

struct ConfigServer {

public:
	ConfigServer(void);

	// Getter functions
	std::string getServerName() const;
	std::string getRoot() const;
	std::map<std::string, unsigned short> getListen() const;
	unsigned long getMaxBodySize() const;
	std::map<unsigned short, std::string> getErrorPages() const;
	std::vector<Location> getLocations() const;

	// Setter functions
	void setRoot(const std::string& root);
	void setListen(const std::string& ipsetress, unsigned short port);
	void setMaxBodySize(unsigned long maxBodySize);
	void setErrorPage(unsigned short code, const std::string& path);
	void setLocation(const Location& location);

private:
	std::string m_serverName;
	std::string m_root;
	std::map<std::string, unsigned short> m_listen;
	unsigned long m_maxBodySize;
	std::map<unsigned short, std::string> m_errorPage;
	std::vector<Location> m_locations;
};

struct ConfigFile {
	std::ifstream stream;
	std::string currentLine;
	std::vector<ConfigServer> servers;
};

std::ostream& operator<<(std::ostream& ostream, const Location& location);
std::ostream& operator<<(std::ostream& ostream, const ConfigServer& configServer);
std::ostream& operator<<(std::ostream& ostream, const ConfigFile& configFile);
