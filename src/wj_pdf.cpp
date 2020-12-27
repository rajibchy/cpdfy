/**
* Copyright (c) 2018, SOW (https://www.safeonline.world). (https://github.com/RKTUXYN) All rights reserved.
* @author {SOW}
* Copyrights licensed under the New BSD License.
* See the accompanying LICENSE file for terms.
*/
#	include "default.h"
#	include "wj_pdf.h"
#	include "pdf_generator.h"
#	include <map>
#	include "v8_util.h"
#	include <nan.h>

std::string to_cstr(v8::Isolate*isolate, v8::Local<v8::Value>val){
	if( val->IsNullOrUndefined() ) return std::string();
	v8::String::Utf8Value str(isolate, val);
	return std::string(*str);
}
void v8_object_loop(v8::Isolate* isolate, const v8::Local<v8::Object>v8_obj, std::map<std::string, std::string>& out_put) {
	v8::Local<v8::Context>ctx = isolate->GetCurrentContext();
	v8::Local<v8::Array> property_names = v8_obj->GetOwnPropertyNames(ctx).ToLocalChecked();
	uint32_t length = property_names->Length();
	for (uint32_t i = 0; i < length; ++i) {
		v8::Local<v8::Value> key = property_names->Get(ctx, i).ToLocalChecked();
		v8::Local<v8::Value> value = v8_obj->Get(ctx, key).ToLocalChecked();
		if (value->IsNullOrUndefined())continue;
		if (key->IsString() && value->IsString()) {
			out_put[to_cstr(isolate, key)] = to_cstr(isolate, value);
		}
	}
}
void generate_pdf(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();
	if (args[0]->IsNullOrUndefined() || !args[0]->IsObject()) {
		throw_js_error(args.GetIsolate(),"Options should be Object");
		return;
	}
	pdf_ext::pdf_generator* pdf_gen = new pdf_ext::pdf_generator();
	v8::Local<v8::Context>ctx = isolate->GetCurrentContext();
	v8::Local<v8::Object> config = Nan::To<v8::Object>(args[0]).ToLocalChecked();
	std::string output_path_str = to_cstr(isolate, config->Get(ctx, v8_str(isolate, "out_path")).ToLocalChecked());
	std::string from_url_str = to_cstr(isolate,config->Get(ctx, v8_str(isolate, "from_url")).ToLocalChecked());
	int is_from_url = FALSE;
	if( from_url_str.length() > 0 ){
		if( output_path_str.length() == 0 ){
			throw_js_error(isolate,"out_path required. If you like to generate pdf from url.");
			return;
		}
		is_from_url = TRUE;
	}
	int rec = -1;
	if( is_from_url == FALSE ) {
		if (args[1]->IsNullOrUndefined() || !args[1]->IsString()) {
			throw_js_error(args.GetIsolate(),"HTML body should be String");
			return;
		}
		v8::Local<v8::Value> v8_global_settings_str = config->Get(ctx, v8_str(isolate, "global_settings")).ToLocalChecked();
		std::map<std::string, std::string>* wgs_settings = new std::map<std::string, std::string>();
		if (!v8_global_settings_str->IsNullOrUndefined() && v8_global_settings_str->IsObject()) {
			v8::Local<v8::Object>v8_global_settings_object = v8::Local<v8::Object>::Cast(v8_global_settings_str);
			v8_object_loop(isolate, v8_global_settings_object, *wgs_settings);
			v8_global_settings_str.Clear();
			v8_global_settings_object.Clear();
		}
		v8::Local<v8::Value> v8_object_settings_str = config->Get(ctx, v8_str(isolate, "object_settings")).ToLocalChecked();
		std::map<std::string, std::string>* wos_settings = new std::map<std::string, std::string>();
		if (!v8_object_settings_str->IsNullOrUndefined() && v8_object_settings_str->IsObject()) {
			v8::Local<v8::Object>v8_object_settings_object = v8::Local<v8::Object>::Cast(v8_object_settings_str);
			v8_object_loop(isolate, v8_object_settings_object, *wos_settings);
			v8_object_settings_str.Clear();
			v8_object_settings_object.Clear();
		}
		rec = pdf_gen->init(FALSE, *wgs_settings, *wos_settings);
		int has_output_path = output_path_str.length() > 0 ? TRUE : FALSE;
		std::string output;
		if (rec < 0) {
			output = std::string(pdf_gen->get_status_msg());
			pdf_gen->dispose();
			delete pdf_gen;
		} else {
			v8::Local<v8::Value> v8_string = args[1];
			std::string cc_string = to_cstr(isolate, v8_string);
			if( has_output_path == FALSE ){
				rec = pdf_gen->generate(cc_string.c_str(), output);
			} else {
				rec = pdf_gen->generate_to_path(cc_string.c_str(), output_path_str.c_str());
			}
			swap_obj(cc_string); v8_string.Clear();
			if (rec < 0) {
				output = std::string(pdf_gen->get_status_msg());
			}else if( has_output_path == FALSE && output.length() == 0){
				rec = -1;
				output = std::string("NO_OUTPUT_DATA_FOUND");
			}
			pdf_gen->dispose();
			delete pdf_gen;
		}
		if( has_output_path == TRUE ) {
			if( rec < 0 ){
				throw_js_error(isolate, output.c_str());
			} else {
				args.GetReturnValue().Set(v8::Number::New(isolate, rec));
			}
		} else {
			if( rec < 0 ) {
				args.GetReturnValue().Set(v8_str(isolate, output.c_str()));
			} else{
				args.GetReturnValue().Set(Nan::CopyBuffer(output.c_str(), output.length()).ToLocalChecked());
			}
		}
		swap_obj(output);
		_free_obj(wgs_settings); _free_obj(wos_settings);
	} else {
		rec = pdf_gen->init(FALSE);
		if (rec < 0) {
			throw_js_error(isolate, pdf_gen->get_status_msg());
		} else {
			rec = pdf_gen->generate_from_url(from_url_str.c_str(), output_path_str.c_str());
			if (rec < 0) {
				throw_js_error(isolate, pdf_gen->get_status_msg());
			}
		}
		pdf_gen->dispose();
		delete pdf_gen;
		if( rec >=0 ){
			args.GetReturnValue().Set(v8::Number::New(isolate, rec));
		}
	}
	swap_obj(output_path_str); swap_obj(from_url_str);
	config.Clear();
	return;
}
void get_http_header(const v8::FunctionCallbackInfo<v8::Value>& args) {
	v8::Isolate* isolate = args.GetIsolate();
	v8::Local<v8::Context> context = isolate->GetCurrentContext( );
	v8::Local<v8::Object> header = v8::Object::New( isolate );
	header->Set( context, v8_str( isolate, "x-wkhtmltopdf-version" ), v8_str(isolate, wkhtmltopdf_version()) ).ToChecked();
	header->Set( context, v8_str( isolate, "accept-ranges" ), v8_str(isolate, "bytes") ).ToChecked();
	header->Set( context, v8_str( isolate, "content-type" ), v8_str(isolate, "application/pdf") ).ToChecked();
	args.GetReturnValue().Set(header);
}