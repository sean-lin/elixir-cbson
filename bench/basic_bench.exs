defmodule DecodBench do
  use Benchfella

  bench "decoder(cbson)", [bson: get_bson()] do
    CBson.decode(bson)
  end
  
  bench "decoder(posion)", [bson: get_bson()] do
    BSON.decode(bson)
  end

  defp get_bson do
    %{
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
      r:  %Bson.Timestamp{ts: 2},
      t: Bson.ObjectId.from_string("52e0e5a10000020003000004")
    } |> Bson.encode
  end
end

defmodule EncodeBench do
  use Benchfella

  bench "encode(cbson)", [bson: get_bson()] do
    CBson.encode(bson)
  end
  
  bench "encode(posion)", [bson: get_posion()] do
    BSON.encode(bson)
  end

  defp get_bson do
    %{
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
      r:  %Bson.Timestamp{ts: 2},
      t: Bson.ObjectId.from_string("52e0e5a10000020003000004")
    }
  end

  defp get_posion() do
    get_bson() |> Bson.encode |> BSON.decode
  end
end

