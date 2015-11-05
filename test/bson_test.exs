Code.require_file "test_helper.exs", __DIR__

defmodule Bson.Test do
  use ExUnit.Case

  doctest Bson
  doctest Bson.ObjectId
  doctest Bson.UTC
end
