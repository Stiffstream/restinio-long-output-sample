#include <restinio/all.hpp>

#include <random>

using namespace std::chrono_literals;

using steady_clock = std::chrono::steady_clock;

namespace asio_ns = restinio::asio_ns;

// Type of output.
using output_t = restinio::chunked_output_t;

// Type of response builder.
using response_t = restinio::response_builder_t<output_t>;

struct response_data {
	asio_ns::io_context & io_ctx_;
	response_t response_;
	int counter_;

	response_data(
		asio_ns::io_context & io_ctx,
		response_t response,
		int counter)
		: io_ctx_{io_ctx}
		, response_{std::move(response)}
		, counter_{counter}
	{}
};

using response_data_shptr = std::shared_ptr<response_data>;

std::string make_buffer(std::size_t size)
{
	std::string buffer;
	buffer.reserve(size);

	char symbols[] = "123456789_";
	while(buffer.size() != size)
		for(auto i = std::begin(symbols);
				i != std::end(symbols) && buffer.size() != size; ++i)
			buffer += *i;

	return buffer;
}

void send_next_portion(response_data_shptr data);

auto make_done_handler(response_data_shptr data) {
	const auto next_timepoint = steady_clock::now() + 1s;
	return [=](const auto & ec) {
		if(!ec) {
			const auto now = steady_clock::now();
			if(now < next_timepoint) {
				// We should delay processing of the next portion.
				auto timer = std::make_shared<asio_ns::steady_timer>(data->io_ctx_);
				timer->expires_after(next_timepoint - now);
				timer->async_wait([timer, data](const auto & ec) {
						if(!ec)
							send_next_portion(data);
					});
			}
			else
				// The next portion should be processed as soon as possible.
				data->io_ctx_.post([data] { send_next_portion(data); });
		}
	};
}

void send_next_portion(response_data_shptr data) {
	data->response_.append_chunk(make_buffer(100*1024));

	if(0 == data->counter_) {
		data->response_.flush();
		data->response_.done();
	}
	else {
		data->counter_ -= 1;
		data->response_.flush(make_done_handler(data));
	}
}

void request_processor(
		asio_ns::io_context & ctx,
		restinio::request_handle_t req) {
	// Start processing of a new request.
	auto data = std::make_shared<response_data>(
			ctx,
			req->create_response<output_t>(),
			6000);

	// Make necessary response headers.
	data->response_
		.append_header(restinio::http_field::server, "RESTinio")
		.append_header_date_field()
		.append_header(
				restinio::http_field::content_type,
				"text/plain; charset=utf-8")
		.flush();

	// Now we can send main response data.
	send_next_portion(data);
}

int main() {
	// Traits for our simple server.
	struct traits_t : public restinio::default_traits_t {
		using logger_t = restinio::shared_ostream_logger_t;
	};

	asio_ns::io_context io_ctx;

	restinio::run(
		io_ctx,
		restinio::on_this_thread<traits_t>()
			.port(8080)
			.address("localhost")
			.write_http_response_timelimit(60s)
			.request_handler([&io_ctx](auto req) {
				// Handle only HTTP GET requests for the root.
				if(restinio::http_method_get() == req->header().method() &&
						"/" == req->header().path()) {
					request_processor(io_ctx, std::move(req));
					return restinio::request_accepted();
				}
				else
					return restinio::request_rejected();
			}));

	return 0;
}

