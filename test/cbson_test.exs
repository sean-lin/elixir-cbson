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

  test "encode zero key" do
    try do
      CBson.encode(%{"a\0b" => 1})
    catch
      a, b ->
        assert {a, b} == {:throw, {:error, {:invalid_key, "a\0b"}}}
    end
    assert CBson.encode(%{"ab" => 1}) == <<13, 0, 0, 0, 16, 97, 98, 0, 1, 0, 0, 0, 0>>
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

    t = [{:a, 1, 3}]
    error = catch_throw CBson.encode(t)
    assert {:error, {:invalid_object_member, {:a, 1, 3}}} == error

    t = [{:a, 1} | %{}]
    error = catch_throw CBson.encode(t)
    assert {:error, {:invalid_keywords, %{}}} == error
  end

  test "regex" do
    t = %{ n:  %Bson.Regex{pattern: "p", opts: "o"},}
    assert t == CBson.encode(t) |> CBson.decode([:return_atom])
  end

  test "double" do
    t = %{d: :nan}
    assert <<16, 0, 0, 0, 1, 100, 0, 0, 0, 0, 0, 0, 0, 248, 127, 0>> == CBson.encode(t)

    t = %{d: :"+inf"}
    assert <<16, 0, 0, 0, 1, 100, 0, 0, 0, 0, 0, 0, 0, 240, 127, 0>> == CBson.encode(t)

    t = %{d: :"-inf"}
    assert <<16, 0, 0, 0, 1, 100, 0, 0, 0, 0, 0, 0, 0, 240, 255, 0>> == CBson.encode(t)

    t = %{a: :nan, b: :"+inf", c: "-inf"}
    assert t == CBson.encode(t) |> CBson.decode([:return_atom])
  end

  test "save struct" do
    t = %{__struct__: ABC, a: 1, b: 2}
    assert CBson.encode(%{key: t}) |> CBson.decode([:return_atom]) == %{key: :maps.remove(:__struct__, t)}
  end

  test "base64encode" do
    bin = <<16, 0, 0, 0, 1, 100, 0, 0, 0, 0, 0, 0, 0, 248, 127, 0>>
    assert Base.encode64(bin) == CBson.nif_b64encode(bin)
  end

  test "objectid" do
    assert "1234567890220200f3abcdef" == (Bson.ObjectId.from_string("1234567890220200f3abcdef") |> to_string())
    assert_raise ArgumentError, fn ->
      Bson.ObjectId.from_string("1234567890220200f3abcde")
    end
    assert_raise ArgumentError, fn ->
      Bson.ObjectId.from_string("1234567890220200f3abcdez")
    end
    assert_raise ArgumentError, fn ->
      %Bson.ObjectId{oid: <<82, 224, 229, 161, 0, 0, 2, 0, 3, 0, 0>>} |> to_string
    end
    assert_raise ArgumentError, fn ->
      %Bson.ObjectId{oid: <<82, 224, 229, 161, 0, 0, 2, 0, 3, 0, 0, 4, 5>>} |> to_string
    end
  end
  test "split" do
    assert {"ab", "cd"} == CBson.nif_split_by_char(<<"00ab", 0, "cd">>, 0, 2)
    assert {"", "cd"} == CBson.nif_split_by_char(<<"00", 0, "cd">>, 0, 2)
    assert {"ab", ""} == CBson.nif_split_by_char(<<"00ab", 0>>, 0, 2)
    assert {"", "cd"} == CBson.nif_split_by_char(<<0, "cd">>, 0, 0)
    assert {"zz", ""} == CBson.nif_split_by_char(<<"zz", 0>>, 0, 0)
    assert_raise ArgumentError, fn ->
      CBson.nif_split_by_char(<<"zz", 0>>, 0, 4)
    end
    assert_raise ArgumentError, fn ->
      CBson.nif_split_by_char(<<"zz", 0, "bb">>, 1111, 0)
    end
  end

  # 需要运行一段时间没有coredump
  @tag :long_term_test
  test "long term packet lower than 2k" do
    Enum.map(1..8, fn _ ->
      {:ok, pid} = Task.Supervisor.start_link()

      Task.Supervisor.async(pid, fn ->
        for _ <- 1..999_999_999 do
          data =
            Enum.reduce(1..5, %{}, fn _, acc ->
              rand_byte = :crypto.strong_rand_bytes(:rand.uniform(5))
              %{aaa: acc, bbb: acc, ccc: rand_byte}
            end)

          for _ <- 1..50 do
            CBson.encode(data)
          end

          :timer.sleep(10)
        end
      end)
    end)

    :timer.sleep(120_000)
  end

  @tag :long_term_test
  test "long term packet larger than 2k" do
    Enum.map(1..8, fn _ ->
      {:ok, pid} = Task.Supervisor.start_link()

      Task.Supervisor.async(pid, fn ->
        for _ <- 1..999_999_999 do
          data =
            Enum.reduce(1..10, %{}, fn _, acc ->
              rand_byte = :crypto.strong_rand_bytes(:rand.uniform(5))
              %{aaa: acc, bbb: acc, ccc: rand_byte}
            end)

          for _ <- 1..10 do
            CBson.encode(data)
          end

          :timer.sleep(10)
        end
      end)
    end)

    :timer.sleep(120_000)
  end

  defp deep(0, acc) do
    acc
  end
  defp deep(n, acc) do
   deep(n - 1, %{"name" => acc})
  end
end
