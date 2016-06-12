defmodule CBsonTest do
  use ExUnit.Case
  doctest CBson

  setup_all do
    term = %{
      a:  -4.230845,
      b:  "hello",
      c:  %{x: -1, y: 2.2001},
      d:  [23, 45, 200],
      eeeeeeeee:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:binary),
        bin:  <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>},
      f:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:function),
        bin:  <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>},
      g:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:uuid),
        bin:  <<49, 0, 0, 0, 4, 66, 83, 79, 78, 0, 38, 0, 0, 0,
              2, 48, 0, 8, 0, 0, 0, 97, 119, 101, 115, 111, 109,
              101, 0, 1, 49, 0, 51, 51, 51, 51, 51, 51, 20, 64,
              16, 50, 0, 194, 7, 0, 0, 0, 0>>},
      h:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:md5),
              bin:  <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>},
      i:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:user),
              bin:  <<49, 0, 0, 0, 4, 66, 83, 79, 78, 0, 38, 0, 0, 0, 2,
              48, 0, 8, 0, 0, 0, 97, 119, 101, 115, 111, 109, 101,
              0, 1, 49, 0, 51, 51, 51, 51, 51, 51, 20, 64, 16, 50,
              0, 194, 7, 0, 0, 0, 0>>},
      j:  %Bson.ObjectId{oid: <<82, 224, 229, 161, 0, 0, 2, 0, 3, 0, 0, 4>>},
      k1: false,
      k2: true,
      l:  Bson.UTC.from_now({1390, 470561, 277000}),
      m:  nil,
      n:  %Bson.Regex{pattern: "p", opts: "o"},
      q1: -2000444000,
      q2: -8000111000222001,
      r:  %Bson.Timestamp{ts: 2},
      s1: :min_key,
      s2: :max_key,
      t: Bson.ObjectId.from_string("52e0e5a10000020003000004")
    }
    bson = Bson.encode(term)
    {:ok, %{term: term, bson: bson}}
  end

  test "decode", ctx do
    ret = CBson.decode(ctx.bson, [:return_atom])
    assert ret == ctx.term
  end
  
  test "decode deep" do
    term = %{a: %{b: %{c: %{d: %{e: %{f: [[[[[[[[[[[[[[[[[[[[[[[[[[%{z: 0}]]]]]]]]]]]]]]]]]]]]]]]]]]}}}}}}
    bson = CBson.encode(term)
    ret = CBson.decode(bson, [:return_atom])
    assert ret == term

    term = deep(300, %{"hello"=> "world"})
    bson = CBson.encode(term)
    ret = CBson.decode(bson)
    assert ret == term
  end

  test "encode", ctx do
    bin = CBson.encode(ctx.term)
    assert bin == ctx.bson 
  end

  test "encode large" do
    bin = :binary.copy("12345678", 253) <> "1234567"
    t = [a: bin, b: %{c: 1}, c: []]
    cbson = CBson.encode(t) |> :erlang.iolist_to_binary
    assert byte_size(cbson) == 2067
  end

  test "decode trailer" do
    t = %{"a"=> 1}
    bin = :binary.copy(CBson.encode(t), 2)
    assert {:has_trailer, ^t, rest} = CBson.decode(bin, [:return_trailer])
    assert t == CBson.decode(rest, [:return_trailer])
  end

  test "encode/decode large" do
    t = Enum.reduce(1..500, %{}, fn x, acc -> Map.put(acc, to_string(x), :binary.copy("12345678910", x)) end)
    assert t == CBson.encode(t) |> CBson.decode
  end

  test "decode cmd" do
    bin = <<92, 0, 0, 0, 16, 99, 111, 110, 110, 101, 99, 116, 105, 111, 110, 73, 100, 0, 26, 22, 0, 0, 8, 117, 112, 
    100, 97, 116, 101, 100, 69, 120, 105, 115, 116, 105, 110, 103, 0, 0, 16, 110, 0, 0, 0, 0, 0, 16, 115, 121, 
    110, 99, 77, 105, 108, 108, 105, 115, 0, 0, 0, 0, 0, 10, 119, 114, 105, 116, 116, 101, 110, 84, 111, 0, 10, 
    101, 114, 114, 0, 1, 111, 107, 0, 0, 0, 0, 0, 0, 0, 240, 63, 0>>
    assert %{ok: 1.0} = CBson.decode(bin, [:return_atom])
  end

  test "decode fail" do
    error = catch_throw CBson.decode <<1, 2, 0, 0>>
    assert {:error, :invalid_bson} == error
  end
  
  test "encode fail" do
    t = %{a: [1, {2, 3}]}
    error = catch_throw CBson.encode(t)
    assert {:error, {:error_term, {2, 3}}} == error
  end

  test "regex" do
    t = %{ n:  %Bson.Regex{pattern: "p", opts: "o"},}
    assert t == CBson.encode(t) |> CBson.decode([:return_atom])
  end

  defp deep(0, acc) do
    acc
  end
  defp deep(n, acc) do
   deep(n - 1, %{"name" => acc}) 
  end
end
