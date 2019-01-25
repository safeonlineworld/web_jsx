/**
* Copyright (c) 2018, SOW (https://www.safeonline.world). (https://github.com/RKTUXYN) All rights reserved.
* @author {SOW}
* Copyrights licensed under the New BSD License.
* See the accompanying LICENSE file for terms.
*/
#include "web_jsx_cgi.h"
#if !(defined(_WIN32) || defined(_WIN64)) && (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
bool is_user_interactive() {
	return false;
};
void print_info() {
	std::cout << "*******************************************************" << std::endl;
};
#else
BOOL is_user_interactive() {
	BOOL bIsUserInteractive = TRUE;
	HWINSTA hWinStation = GetProcessWindowStation();
	if (hWinStation != NULL) {
		USEROBJECTFLAGS uof = { 0 };
		if (GetUserObjectInformation(hWinStation, UOI_FLAGS, &uof, sizeof(USEROBJECTFLAGS), NULL) && ((uof.dwFlags & WSF_VISIBLE) == 0)) {
			bIsUserInteractive = FALSE;
		}
	}
	return bIsUserInteractive;
};
WORD get_current_console_color(HANDLE hConsole) {
	CONSOLE_SCREEN_BUFFER_INFO *ConsoleInfo = new CONSOLE_SCREEN_BUFFER_INFO();
	GetConsoleScreenBufferInfo(hConsole, ConsoleInfo);
	WORD OriginalColors = ConsoleInfo->wAttributes;
	return OriginalColors;
};
void print_info() {
	/*
	1_____Blue				A______Light Green
	2_____Green				B______Light Aqua
	3_____Aqua				C______Light Red
	4_____Red				D______Light Purple
	5_____Purple			E______Light Yellow
	6_____Yellow			F______Bright White
	7_____White
	8_____Gray
	9_____Light Blue
	0_____Black
	*/
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD orginal_color = get_current_console_color(hConsole);
	SetConsoleTextAttribute(hConsole, FOREGROUND_LIGHT_RED);
	std::cout << "*******************************************************" << std::endl;
	SetConsoleTextAttribute(hConsole, FOREGROUND_LIGHT_GREEN);
	std::cout << "Web ";
	SetConsoleTextAttribute(hConsole, FOREGROUND_LIGHT_RED);
	std::cout << "Extension Handler ";
	SetConsoleTextAttribute(hConsole, FOREGROUND_LIGHT_GREEN);
	std::cout << "*.jsx & *.jsxh;" << "\r\n";
	SetConsoleTextAttribute(hConsole, FOREGROUND_DARK_YELLOW);
	std::cout << "Version: 1.0.0.1" << "\r\n";
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
	std::cout << "Repo:https://github.com/RKTUXYN/web_jsx_cgi" << "\r\n";
	SetConsoleTextAttribute(hConsole, FOREGROUND_LIGHT_GREEN);
	std::cout << "Create nested template in your web application.\r\n";
	std::cout << "Run javascript into CLI or Web Back-end\r\n";
	std::cout << "Supported Web Server IIS, Apache & Nginx \r\n";
	std::cout << "Extension-> .jsx (Mixed handler (javascript & html)) \r\n";
	std::cout << "Extension-> .jsxh (Raw javascript handler)\r\n";
	SetConsoleTextAttribute(hConsole, FOREGROUND_LIGHT_RED);
	std::cout << "*******************************************************";
	fflush(stdout);
	SetConsoleTextAttribute(hConsole, orginal_color);
};
#endif//!_WIN32||_WIN64
void obj_insert(std::map<std::string, std::string>&from_obj, const char* prop, std::map<std::string, std::map<std::string, std::string>>&to_obj) {
	to_obj[prop] = from_obj;
};
void get_request_object(std::map<std::string, std::string>&request, std::map<std::string, std::string>&query_string, req_method&method) {
	request["method"] = method == req_method::GET ? "GET" : "POST";
	if (method == req_method::POST) {
		request["content_length"] = get_env_c("CONTENT_LENGTH");
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
	request["page_path"] = get_env_c("PATH_INFO");
	request["content_type"] = get_content_type();
	request["cookie"] = get_env_c("HTTP_COOKIE");
	request["user_agent"] = get_env_c("HTTP_USER_AGENT");
	request["accept"] = get_env_c("HTTP_ACCEPT");
	request["accept_encoding"] = get_env_c("HTTP_ACCEPT_ENCODING");
};
void get_global_obj(std::map<std::string, std::string>& global, std::string&root_dir) {
	replace_back_slash(root_dir);
	global["root_dir"] = root_dir;
	global["host"] = get_env_c("HTTP_HOST");
	global["remote_addr"] = get_env_c("REMOTE_ADDR");
	global["server_protocol"] = get_env_c("SERVER_PROTOCOL");
	global["https"] = get_env_c("HTTPS");
};
void get_user_obj(std::map<std::string, std::string>& user_info, std::map<std::string, std::string>& global) {
	user_info["name"] = "";
	user_info["is_authenticated"] = "false";
	user_info["role"] = "";
};
void write_header(const char* ct) {
	std::cout << "Content-Type:" << ct << "\n";
	std::cout << "Accept-Ranges:bytes\n";
	std::cout << "X-Powered-By:safeonline.world\n";
	std::cout << "X-Process-By:web_jsx_cgi\n";
};
void not_found_response(const char* content_type) {
	std::string resp(get_env_c("SERVER_PROTOCOL"));
	resp.append(" 404 Not found\n");
	std::cout << resp;
	write_header(content_type);
	std::cout << "\r\n";
};
void run__js_scrip(const char* content_type, std::string&ex_dir, std::string&root_dir, req_method&method, template_result& tr) {
	auto ctx = new std::map<std::string, std::map<std::string, std::string>>();
	auto req_obj = new std::map<std::string, std::string>();
	std::map<std::string, std::string>*query_string = new std::map<std::string, std::string>();
	read_query_string(*query_string);
	get_request_object(*req_obj, *query_string, method);
	auto global_obj = new std::map<std::string, std::string>();
	get_global_obj(*global_obj, root_dir);
	auto user_inf = new std::map<std::string, std::string>();
	get_user_obj(*user_inf, *global_obj);
	obj_insert(*req_obj, "request", *ctx);
	obj_insert(*global_obj, "global", *ctx);
	obj_insert(*user_inf, "user", *ctx);
	std::future<int> result = std::async(std::launch::async | std::launch::deferred, [&]() {
		try {
		sow_web_jsx::js_compiler::run_template_x(*ctx, ex_dir.c_str(), tr);
		} catch (std::exception&e) {
			write_header(content_type);
			std::cout << "\r\n";
			std::cout << e.what();
		}
		return 1;
	});
	result.get();
	delete req_obj, delete global_obj; delete ctx; delete user_inf; delete query_string;
	std::string().swap(tr.t_source);
};
void prepare_response(const char* content_type, const char*path_translated, char* execute_path, req_method&method) {
	web_extension ext = get_request_extension(path_translated);
	if (ext == web_extension::UNKNOWN) {
		write_header(content_type);
		std::cout << "\r\n";
		std::cout << "Unsupported extension....";
		std::cout << path_translated;
		fflush(stdout);
		return;
	}
	auto ex_dir = new std::string();
	auto ex_name = new std::string();
	request_file_info(execute_path, *ex_dir, *ex_name);
	ex_dir->append("\\");
	auto root_dir = new std::string();
	const char* path_info = get_env_c("PATH_INFO");
	server_physical_path(path_translated, path_info, *root_dir);
	root_dir->append("\\");
	template_result* tr = new template_result();
	tr->remove_new_line = true;
	if (ext == web_extension::JSXH) {
		std::stringstream stream;
		sow_web_jsx::js_write_header(stream);
		if (sow_web_jsx::read_file(path_translated, stream, false) < 0) {
			not_found_response(content_type);
			return;
		};
		sow_web_jsx::js_write_footer(stream);
		tr->t_source = stream.str();
		std::stringstream().swap(stream);
		run__js_scrip(content_type, *ex_dir, *root_dir, method, *tr);
		if (tr->is_error == true) {
			write_header(content_type);
			std::cout << "\r\n";
			std::cout << tr->err_msg.c_str();
			fflush(stdout);
		}
	}
	else {
		parser_settings* ps = new parser_settings();
		ps->dir = root_dir->c_str();
		ps->page_path = path_info;
		ntemplate_parse_x(*ps, *tr);
		if (tr->is_error == true) {
			if (tr->err_msg == "_NOT_FOUND_") {
				not_found_response(content_type);
			}
			else {
				write_header(content_type);
				std::cout << "\r\n";
				std::cout << tr->err_msg.c_str();
				fflush(stdout);
			}
		}
		else if (tr->is_script_template == true) {
			run__js_scrip(content_type, *ex_dir, *root_dir, method, *tr);
			if (tr->is_error == true) {
				write_header(content_type);
				std::cout << "\r\n";
				std::cout << tr->err_msg.c_str();
				fflush(stdout);
			}
		}
		else {
			write_header(content_type);
			std::cout << "\r\n";
			SET_BINARY_MODE(stdout);
			std::cout << tr->t_source.c_str();
			std::string().swap(tr->t_source);
			fflush(stdout);
		}
		delete ps;
	}
	delete tr; 
	delete ex_dir; delete ex_name; delete root_dir;

};
void get_args(int argc, char *argv[], std::vector<char*>&args) {
	for (int i = 2; i < argc; i++) {
		args.push_back(argv[i]);
	}
};
// No external changes needed. 
void prepare_console_response(int argc, char *argv[]) {
	//4:37 AM 12/13/2018
	auto exec_path = new std::string();
#if defined(__WEB_JSX_PUBLISH)
	if ( get_env_path(*exec_path) < 0 ) {
		delete exec_path;
		std::cout << "Please add web_jsx bin path into environment variable Path!!!\r\n";
		fflush(stdout);
		return;
	}
#else
	exec_path->append(get_env_c("web_jsx_cgi"));
#endif
	exec_path->append("\\");
	const char*may_be_path = const_cast<const char*>(argv[1]);
	web_extension ext = get_request_extension(may_be_path);
	if (ext == web_extension::JSX) {
		std::cout << "This extension not allowed for CLI";
		std::cout << " ==> ";
		std::cout << may_be_path;
		fflush(stdout);
		return;
	}
	auto root_dir = new std::string(get_current_working_dir());
	root_dir->append("\\");
	std::stringstream stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);
	if (ext == web_extension::RAW_SCRIPT || ext == web_extension::UNKNOWN) {
		sow_web_jsx::js_write_console_header(stream);
		stream << may_be_path;
	}
	else {
		auto scrip_path = new std::string(root_dir->data());
		scrip_path->append(may_be_path);
		const char* scrip_path_c = scrip_path->data();
		if (__file_exists(scrip_path_c) == false) {
			std::cout << "No file found in : " << scrip_path_c;
			delete root_dir; delete scrip_path; delete exec_path;
			fflush(stdout);
			return;
		}
		delete scrip_path;
		sow_web_jsx::js_write_console_header(stream);
		if (sow_web_jsx::read_file(may_be_path, stream, false) < 0) {
			std::cout << "Unable to read file : " << scrip_path_c;
			std::stringstream().swap(stream);
			delete root_dir; delete exec_path;
			fflush(stdout);
			return;
		};
	}
	
	sow_web_jsx::js_write_console_footer(stream);
	std::vector<char*>*argumnets = new std::vector<char*>();
	get_args(argc, argv, *argumnets);
	auto arg_array = new std::string("");
	json_array_stringify_s(*argumnets, *arg_array);
	auto script_source = new std::string(stream.str());
	std::stringstream().swap(stream);
	if (ext == web_extension::RAW_SCRIPT) {
		std::future<int> result = std::async(std::launch::async | std::launch::deferred, [=]() {
			try {
				sow_web_jsx::js_compiler::run_console_script_x(exec_path->data(), script_source->data(), root_dir->data(), "RAW_SCRIPT", arg_array->data());
			} catch (std::exception&e) {
				std::cout << e.what();
			}
			return 1;
		});
		result.get();
	}
	else {
		std::future<int> result = std::async(std::launch::async | std::launch::deferred, [=]() {
			try {
				run_console_script_x(exec_path->data(), script_source->data(), root_dir->data(), may_be_path, arg_array->data());
			} catch (std::exception&e) {
				std::cout << e.what();
			}
			return 1;
		});
		result.get();
	}
	delete argumnets; delete script_source;
	delete root_dir; delete arg_array; delete exec_path;
	fflush(stdout);
};
int main(int argc, char *argv[], char*envp[]) {
	std::ios::sync_with_stdio(false);
	try {
		if (is_user_interactive() == TRUE) {
			if (argc > 1) {
				prepare_console_response(argc, argv);
				return EXIT_SUCCESS;
			}
			print_info();
			return EXIT_SUCCESS;
		}
		const char* CONTENT_TYPE = get_content_type();
		const char*PATH_TRANSLATED = get_env_c("PATH_TRANSLATED");
		if (__file_exists(PATH_TRANSLATED) == false) {
			not_found_response(CONTENT_TYPE);
			return EXIT_FAILURE;
		};
		req_method method = determine_req_method();
		if (method == req_method::UNSUPPORTED) {
			write_header(CONTENT_TYPE);
			std::cout << "\r\n";
			std::cout << "This method " << get_env("REQUEST_METHOD") << "not supported!!!" << "\r\n";
			return EXIT_FAILURE;
		}
		prepare_response(CONTENT_TYPE, PATH_TRANSLATED, argv[0], method);
	} catch (std::exception&e) {
		write_header(get_content_type());
		std::cout << "\r\n";
		std::cout << e.what() << "\r\n";
		return EXIT_FAILURE;
	}
};
