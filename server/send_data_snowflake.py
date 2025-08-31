import pandas as pd
import duckdb
from snowflake.connector.pandas_tools import write_pandas
# 1. Connect to DuckDB and fetch data into a Pandas DataFrame
con_duck = duckdb.connect('data/processed_weather.duckdb')
df = con_duck.execute("SELECT * FROM processed_intervals").fetchdf()
print(df.head())
con_duck.close()


# 2. Connect to Snowflake
import snowflake.connector
con_snow = snowflake.connector.connect(
    user='aksjdfhl',
    password='aksjdhflkashdf',
    account='es31808.europe-west3.gcp',
    warehouse='COMPUTE_WH',
    database='KASSELWEATHER',
    schema='PUBLIC'
)

# 3. Write the DataFrame to a Snowflake table
# This function will create or replace the table and load the data
success, nchunks, nrows, output = write_pandas(
    conn=con_snow,
    df=df,
    table_name='KASSEL_WEATHER_TEST',
    auto_create_table=True,
    overwrite=True
)

print(f"Success: {success}, Rows loaded: {nrows}")
con_snow.close()