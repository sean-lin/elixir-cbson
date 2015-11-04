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
      q1: -2000444000,
      q2: -8000111000222001,
      #r:  %Bson.Timestamp{inc: 1, ts: 2},
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
    bson = Bson.encode(term)
    ret = CBson.decode(bson, [:return_atom])
    assert ret == term

    term = deep(300, %{"hello"=> "world"})
    bson = Bson.encode(term)
    ret = CBson.decode(bson)
    assert ret == term
  end

  test "tc", ctx do
    _ = Bson.decode(ctx.bson) 
    {t, _} = :timer.tc(fn -> CBson.decode(ctx.bson, [:return_atom]) end)
    IO.puts "CBSON: #{t}"
    {t, _} = :timer.tc(fn -> CBson.decode(ctx.bson) end)
    IO.puts "CBSON not atom: #{t}"
    {t, _} = :timer.tc(fn -> Bson.decode(ctx.bson) end)
    IO.puts "BSON: #{t}"
  end

  test "encode", ctx do
    bin = CBson.encode(ctx.term)
    rbin = Bson.encode(ctx.term)
    assert bin == rbin
    assert ctx.term == CBson.decode(bin, [:return_atom]) 
    
    {t, _} = :timer.tc(fn -> CBson.encode(ctx.term) end)
    IO.puts "encode CBSON: #{t}"
    
    {t, _} = :timer.tc(fn -> Bson.encode(ctx.term) end)
    IO.puts "encode BSON: #{t}"
  end

  test "encode large" do
    bin = :binary.copy("12345678", 253) <> "1234567"
    t = [a: bin, b: %{c: 1}]
    bson = Bson.encode(t)
    cbson = CBson.encode(t) |> :erlang.iolist_to_binary
    assert bson == cbson
  end

  defp deep(0, acc) do
    acc
  end
  defp deep(n, acc) do
   deep(n - 1, %{"name" => acc}) 
  end
end
