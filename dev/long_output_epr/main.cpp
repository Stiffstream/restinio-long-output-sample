#include <restinio/all.hpp>
#include <restinio/router/easy_parser_router.hpp>

using namespace std::chrono_literals;

// Type of clock to be used in the code.
using steady_clock = std::chrono::steady_clock;

// Just for convenience.
namespace asio_ns = restinio::asio_ns;

// Short alias for express-like router.
using router_t = restinio::router::easy_parser_router_t;

// Type of output.
using output_t = restinio::chunked_output_t;

// Type of response builder.
using response_t = restinio::response_builder_t<output_t>;

// Description of processed response.
struct response_data {
	asio_ns::io_context & io_ctx_;
	std::size_t chunk_size_;
	response_t response_;
	std::size_t counter_;

	response_data(
		asio_ns::io_context & io_ctx,
		std::size_t chunk_size,
		response_t response,
		std::size_t counter)
		: io_ctx_{io_ctx}
		, chunk_size_{chunk_size}
		, response_{std::move(response)}
		, counter_{counter}
	{}
};

// Just an alias for shared_ptr.
using response_data_shptr = std::shared_ptr<response_data>;

std::string make_buffer(std::size_t size) {
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
	data->response_.append_chunk(make_buffer(data->chunk_size_));

	if(1u == data->counter_) {
		data->response_.flush();
		data->response_.done();
	}
	else {
		data->counter_ -= 1u;
		data->response_.flush(make_done_handler(data));
	}
}

void request_processor(
		asio_ns::io_context & ctx,
		std::size_t chunk_size,
		std::size_t count,
		restinio::request_handle_t req) {
	// Start processing of a new request.
	auto data = std::make_shared<response_data>(
			ctx,
			chunk_size,
			req->create_response<output_t>(),
			count);

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

auto make_router(asio_ns::io_context & ctx) {
	using namespace restinio::router::easy_parser_router;

	auto router = std::make_unique<router_t>();

	struct distribution_params
	{
		std::size_t chunk_size_{100u*1024u};
		std::size_t count_{10000u};
	};
	struct chunk_size { std::uint32_t c_{1u}, m_{1u}; };

	router->http_get(
		path_to_params(
			produce<distribution_params>(
				exact("/"),
				maybe(
					produce<chunk_size>(
						non_negative_decimal_number_p<std::uint32_t>()
							>> &chunk_size::c_,
						maybe(
							produce<std::uint32_t>(
								alternatives(
									caseless_symbol_p('b') >> just_result(1u),
									caseless_symbol_p('k') >> just_result(1024u),
									caseless_symbol_p('m') >> just_result(1024u * 1024u)
								)
							) >> &chunk_size::m_
						)
					) >> convert(
							[](auto cs) { return std::size_t{cs.c_} * cs.m_; })
						>> &distribution_params::chunk_size_,
					maybe(
						exact("/"),
						non_negative_decimal_number_p<std::size_t>()
							>> &distribution_params::count_
					)
				)
			)
		),
		[&ctx](const auto & req, const auto & params )
		{
			if(0u != params.chunk_size_) {
				request_processor(
						ctx,
						params.chunk_size_,
						params.count_,
						std::move(req));
				return restinio::request_accepted();
			}
			else
				return restinio::request_rejected();
		});

	return router;
}

int main() {
	// Traits for our simple server.
	struct traits_t : public restinio::default_single_thread_traits_t {
		using logger_t = restinio::single_threaded_ostream_logger_t;
		using request_handler_t = router_t;
	};

	asio_ns::io_context io_ctx;

	restinio::run(
		io_ctx,
		restinio::on_this_thread<traits_t>()
			.port(8080)
			.address("localhost")
			.write_http_response_timelimit(60s)
			.request_handler(make_router(io_ctx)));

	return 0;
}

