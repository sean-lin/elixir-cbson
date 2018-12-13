defmodule JsonExtTest do
  use ExUnit.Case

  doctest Bson.JsonExt

  setup_all do
    term = %{
      a: -4.230845,
      b: "hello",
      c: %{x: -1, y: 2.2001},
      d: [23, 45, 200],
      eeeeeeeee: %Bson.Bin{
        subtype: Bson.Bin.subtyx(:binary),
        bin: <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>
      },
      f: %Bson.Bin{
        subtype: Bson.Bin.subtyx(:function),
        bin: <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>
      },
      g: %Bson.Bin{
        subtype: Bson.Bin.subtyx(:uuid),
        bin:
          <<49, 0, 0, 0, 4, 66, 83, 79, 78, 0, 38, 0, 0, 0, 2, 48, 0, 8, 0, 0, 0, 97, 119, 101,
            115, 111, 109, 101, 0, 1, 49, 0, 51, 51, 51, 51, 51, 51, 20, 64, 16, 50, 0, 194, 7, 0,
            0, 0, 0>>
      },
      h: %Bson.Bin{
        subtype: Bson.Bin.subtyx(:md5),
        bin: <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>
      },
      i: %Bson.Bin{
        subtype: Bson.Bin.subtyx(:user),
        bin:
          <<49, 0, 0, 0, 4, 66, 83, 79, 78, 0, 38, 0, 0, 0, 2, 48, 0, 8, 0, 0, 0, 97, 119, 101,
            115, 111, 109, 101, 0, 1, 49, 0, 51, 51, 51, 51, 51, 51, 20, 64, 16, 50, 0, 194, 7, 0,
            0, 0, 0>>
      },
      j: %Bson.ObjectId{oid: <<82, 224, 229, 161, 0, 0, 2, 0, 3, 0, 0, 4>>},
      k1: false,
      k2: true,
      l: Bson.UTC.from_now({1390, 470_561, 277_000}),
      m: nil,
      q1: -2_000_444_000,
      q2: -8_000_111_000_222_001,
      r: %Bson.Timestamp{ts: 2},
      s1: :min_key,
      s2: :max_key,
      t: Bson.ObjectId.from_string("52e0e5a10000020003000004")
    }

    json = %{
      a: -4.230845,
      b: "hello",
      c: %{x: -1, y: 2.2001},
      d: [23, 45, 200],
      eeeeeeeee: "yAzwgWRaOMYiAAA=",
      f: "yAzwgWRaOMYiAAA=",
      g: "MQAAAARCU09OACYAAAACMAAIAAAAYXdlc29tZQABMQAzMzMzMzMUQBAyAMIHAAAAAA==",
      h: "yAzwgWRaOMYiAAA=",
      i: "MQAAAARCU09OACYAAAACMAAIAAAAYXdlc29tZQABMQAzMzMzMzMUQBAyAMIHAAAAAA==",
      j: "52e0e5a10000020003000004",
      k1: false,
      k2: true,
      l: 1_390_470_561_277,
      m: nil,
      q1: -2_000_444_000,
      q2: -8_000_111_000_222_001,
      r: 2,
      s1: :min_key,
      s2: :max_key,
      t: "52e0e5a10000020003000004"
    }

    bson = Bson.encode(term)
    {:ok, %{term: term, bson: bson, json: json}}
  end

  test "decode", ctx do
    ret = CBson.decode(ctx.bson, [:return_json, :return_atom])
    assert ret == ctx.json
  end
end
