defmodule CBson do
  @on_load {:init, 0}

  def init do
    path = :filename.join(:code.priv_dir(:cbson), 'bson_nif')
    :ok = :erlang.load_nif(path, 0)
  end

  def encode(data) do
    encode(data, [])
  end

  def decode(data) do
    decode(data, [])
  end

  def decode(data, opts) when is_binary(data) and is_list(opts) do
    case nif_decode_init(data, opts) do
      {:error, _} = error ->
        throw(error)
      {:iter, decoder, val, objs, curr} ->
        IO.inspect "loop"
        decode_loop(data, decoder, val, objs, curr);
      bson ->
        bson
    end
  end
  def decode(data, opts) when is_list(data) do
    decode(:erlang.iolist_to_binary(data), opts)
  end

  defp decode_loop(data, decoder, val, objs, curr) do
    case nif_decode_iter(data, decoder, val, objs, curr) do
      {:error, _} = error ->
        throw(error);
      {:iter, decoder, val, objs, curr} ->
        decode_loop(data, decoder, val, objs, curr);
      bson ->
        bson
    end
  end
  
  def encode(data, opts) when is_list(opts) do
    case nif_encode_init(data, opts) do
      {:error, _} = error ->
        throw(error)
      iolist ->
        iolist
    end
  end
  
  defp nif_decode_init(_data, _opts) do
    exit(:nif_library_not_loaded)
  end

  defp nif_decode_iter(_data, _decoder, _, _, _) do
    exit(:nif_library_not_loaded)
  end

  def nif_encode_init(_data, _options) do
    exit(:nif_library_not_loaded)
  end

  def nif_encode_iter(_encoder, _stack) do
    exit(:nif_library_not_loaded)
  end
end
