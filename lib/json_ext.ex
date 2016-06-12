defmodule Bson.JsonExt do
  @moduledoc """
  This module provides two helper methods `dump` and `load`
  methods and provide explicit BSON conversion to and from json compatible Elixir terms.  
  This allows for specialized encoding and decoding of BSON documents into 
  `Mongo Extended JSON <http://www.mongodb.org/display/DOCS/Mongo+Extended+JSON>`_'s *Strict* mode.  
  This lets you encode / decode BSON documents to json even when
  they use special BSON types.

  Usage:
  ```

    iex> term = %{
    ...> a:  -4.230845,
    ...> b:  "hello",
    ...> c:  %{x: -1, y: 2.2001},
    ...> d:  [23, 45, 200],
    ...> eeeeeeeee:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:binary),
    ...>               bin:  <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>},
    ...> f:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:function),
    ...>               bin:  <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>},
    ...> g:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:uuid),
    ...>               bin:  <<49, 0, 0, 0, 4, 66, 83, 79, 78, 0, 38, 0, 0, 0,
    ...>                       2, 48, 0, 8, 0, 0, 0, 97, 119, 101, 115, 111, 109,
    ...>                       101, 0, 1, 49, 0, 51, 51, 51, 51, 51, 51, 20, 64,
    ...>                       16, 50, 0, 194, 7, 0, 0, 0, 0>>},
    ...> h:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:md5),
    ...>               bin:  <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>},
    ...> i:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:user),
    ...>               bin:  <<49, 0, 0, 0, 4, 66, 83, 79, 78, 0, 38, 0, 0, 0, 2,
    ...>                       48, 0, 8, 0, 0, 0, 97, 119, 101, 115, 111, 109, 101,
    ...>                       0, 1, 49, 0, 51, 51, 51, 51, 51, 51, 20, 64, 16, 50,
    ...>                       0, 194, 7, 0, 0, 0, 0>>},
    ...> j:  %Bson.ObjectId{oid: <<82, 224, 229, 161, 0, 0, 2, 0, 3, 0, 0, 4>>},
    ...> k1: false,
    ...> k2: true,
    ...> l:  Bson.UTC.from_now({1390, 470561, 277000}),
    ...> m:  nil,
    ...> q1: -2000444000,
    ...> q2: -8000111000222001,
    ...> s1: :min_key,
    ...> s2: :max_key,
    ...> t: Bson.ObjectId.from_string("52e0e5a10000020003000004")
    ...> }
    ...> json = Bson.JsonExt.dump(term)
    %{
      "a"  => -4.230845,
      "b"  => "hello",
      "c"  => %{"x" => -1, "y" => 2.2001},
      "d"  => [23, 45, 200],
      "e"  => %{"$type" => "00", "$bin" => "zwgWRaOMYiAAA="},
      "f"  => %{"$type" => "01", "$bin" => "zwgWRaOMYiAAA="},
      "g"  => %{"$type" => "03", 
                "$bin" => "MQAAAARCU09OACYAAAACMAAIAAAAYXdlc29tZQABMQAzMzMzMzMUQBAyAMIHAAAAAA=="},
      "h"  => %{"$type" => "05", "$bin" => "zwgWRaOMYiAAA="},
      "i"  => %{"$type" => "80", 
                "$bin" => "MQAAAARCU09OACYAAAACMAAIAAAAYXdlc29tZQABMQAzMzMzMzMUQBAyAMIHAAAAAA=="},
      "j"  => %{"$oid" => "52e0e5a10000020003000004"},
      "k1" => false,
      "k2" => true,
      "l"  => %{"$date" => %{"$numberLong" => "1390470561277"}},
      "m"  => nil,
      "q1" => -2000444000,
      "q2" => -8000111000222001,
      "s1" => %{"$min_key" => 1},
      "s2" => %{"$max_key" => 1},
      "t"  => %{"$oid" => "52e0e5a10000020003000004"}
    }
    ...> bson = Bson.JsonExt.load(json)
    ...> # assert that one by one all decoded element are identical to the original
    ...> Enum.all? term, fn({k, v}) -> assert Map.get(bson, k) == v end
    true

  ```
  
  see `dump/1` and `load/1`
  """

  def dump(bson)
  def dump(n) when is_number(n), do: n
  def dump(str) when is_binary(str), do: str
  def dump(atom) when is_atom(atom), do: atom
  def dump(%Bson.ObjectId{oid: oid}) do
    %{"$oid" => Bson.hex(oid)|>String.downcase}
  end
  def dump(%Bson.UTC{ms: ms}) do
    %{"$date" => %{"$numberLong" => Integer.to_string(ms)}}
  end
  def dump(%Bson.Bin{bin: bin, subtype: subtype}) do
    %{"$binary"=> Base.encode64(bin), "$type" => xsubty(subtype)}
  end
  def dump(bson) when is_map(bson) do
    :maps.map(fn _k, v -> dump(v) end, bson)
  end
  def dump([]), do: []
  def dump([{_, _}|_] = bson) do
    :lists.map(fn {k, v} -> {k, dump(v)} end, bson)
    |> :maps.from_list
  end
  def dump(bson) when is_list(bson) do
    :lists.map(&dump/1, bson)
  end
  def dump(:min_key), do: %{"$minKey" => 1}
  def dump(:max_key), do: %{"$maxKey" => 1}

  def load(json)
  def load(n) when is_number(n), do: n
  def load(str) when is_binary(str), do: str
  def load(atom) when is_atom(atom), do: atom
  def load(json) when is_list(json) do
    :lists.map(&load/1, json)
  end
  def load(%{"$oid" => oid}) do
    Bson.ObjectId.from_string(oid)
  end
  def load(%{"$date" => %{"$numberLong" => ms}}) do
    %Bson.UTC{ms: String.to_integer(ms)}
  end
  def load(%{"$binary" => bin, "$type" => type}) do
    %Bson.Bin{bin: Base.decode64!(bin), subtype: String.to_integer(type, 16)}
  end
  def load(%{"$numberLong" => n}) do
    String.to_integer(n)
  end
  def load(json) when is_map(json) do
    :maps.map(fn _k, v -> load(v) end, json)
  end

  # module static binary
  defp xsubty(0x00), do: "00"
  defp xsubty(0x01), do: "01"
  defp xsubty(0x02), do: "02"
  defp xsubty(0x03), do: "03"
  defp xsubty(0x04), do: "04"
  defp xsubty(0x05), do: "05"
  defp xsubty(0x80), do: "80"
end

