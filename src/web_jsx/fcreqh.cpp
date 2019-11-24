#include "fcreqh.h"
#if defined(FAST_CGI_APP)
const char * web_jsx_cgi::fcgi_request::freq_env_c(const char* var_name, char **envp) {
	char *env_val = FCGX_GetParam(var_name, envp);
	if (((env_val != NULL) && (env_val[0] == '\0')) || env_val == NULL) {
		return "";
	}
	return const_cast<const char*>(env_val);
};
std::string read_requestHeader(char **envp) {
	char **env = envp;
	std::string str("");
	while (*(++env)) {
		auto en = *env;
		str.append(en).append("~");
	}
	return str;
}
void web_jsx_cgi::fcgi_request::get_global_obj(std::map<std::string, std::string>& global, std::string&root_dir, const char*app_path, const char*env_path, char **envp) {
	replace_back_slash(root_dir);
	global["root_dir"] = root_dir;
	global["host"] = freq_env_c("HTTP_HOST", envp);
	global["remote_addr"] = freq_env_c("REMOTE_ADDR", envp);
	global["server_protocol"] = freq_env_c("SERVER_PROTOCOL", envp);
	global["https"] = freq_env_c("HTTPS", envp);
	global["app_path"] = app_path;
	global["env_path"] = env_path;
};
void web_jsx_cgi::fcgi_request::get_request_object(std::map<std::string, std::string>&request, std::map<std::string, std::string>&query_string, req_method&method, const char*content_type, char **envp) {
	request["method"] = method == req_method::GET ? "GET" : "POST";
	if (method == req_method::POST) {
		request["content_length"] = freq_env_c("CONTENT_LENGTH", envp);
	}
	else {
		request["pay_load"] = "[]";
		request["content_length"] = "0";
	}
	if (!query_string.empty()) {
		std::string* quer_str = new std::string("");
		json_obj_stringify(query_string, *quer_str);
		request["query_string"] = quer_str->c_str();
		free(quer_str);
	}
	else {
		request["query_string"] = "{}";
	}
	request["header"] = read_requestHeader(envp);
	request["page_path"] = freq_env_c("PATH_INFO", envp);
	request["content_type"] = content_type;
	request["cookie"] = freq_env_c("HTTP_COOKIE", envp);
	request["user_agent"] = freq_env_c("HTTP_USER_AGENT", envp);
	request["accept"] = freq_env_c("HTTP_ACCEPT", envp);
	request["accept_encoding"] = freq_env_c("HTTP_ACCEPT_ENCODING", envp);
}

void web_jsx_cgi::fcgi_request::not_found_response(const char* content_type, char **envp) {
	std::cout << "Content-Type:" << content_type << H_N_L;
	std::cout << "Accept-Ranges:bytes" << H_N_L;
	std::cout << "X-Powered-By:safeonline.world" << H_N_L;
	std::cout << "X-Process-By:web_jsx" << H_N_L;
	std::cout << "Status: 404 Not found" << H_N_L;
	std::cout << "\r\n";
}
#endif//!FAST_CGI_APP