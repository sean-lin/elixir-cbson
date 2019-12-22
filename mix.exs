defmodule Mix.Tasks.Compile.Nif do
  def run(_) do
    cp = Mix.Project.compile_path()
    priv_path = Path.join([cp, "..", "priv"])
    File.mkdir_p(priv_path)
    File.mkdir_p("priv")

    if Mix.shell().cmd("PRIV_PATH=#{priv_path} make nif") != 0 do
      raise Mix.Error, message: "could not run `make nif`."
    end

    File.copy(Path.join([priv_path, "bson_nif.so"]), "priv")
    :ok
  end
end

defmodule Cbson.Mixfile do
  use Mix.Project

  @version "0.1.0"

  def project do
    [
      app: :cbson,
      name: :cbson,
      version: @version,
      elixir: "~> 1.7",
      package: package(),
      docs: docs(),
      description: description(),
      source_url: "https://github.com/sean-lin/elixir-cbson",
      build_embedded: Mix.env() == :prod,
      start_permanent: Mix.env() == :prod,
      compilers: [:nif | Mix.compilers()],
      deps: deps()
    ]
  end

  # Configuration for the OTP application
  #
  # Type "mix help compile.app" for more information
  def application do
    [applications: [:logger]]
  end

  defp package do
    [
      name: :cbson,
      maintainers: [],
      licenses: ["MIT"],
      files: ["lib/*", "src/*", "priv/*", "mix.exs", "README*", "LICENSE*"],
      links: %{
        "GitHub" => "https://github.com/sean-lin/elixir-cbson"
      }
    ]
  end

  defp docs do
    [
      extras: ["README.md"],
      main: "readme",
      source_ref: "v#{@version}",
      source_url: "https://github.com/sean-lin/elixir-cbson"
    ]
  end

  defp description do
    "BSON NIF for Elixir/Erlang language http://bsonspec.org"
  end

  defp deps do
    [
      {:ex_doc, ">= 0.0.0", only: :dev, runtime: false},
      {:benchfella, "~> 0.3.0", only: :bench}
    ]
  end
end
