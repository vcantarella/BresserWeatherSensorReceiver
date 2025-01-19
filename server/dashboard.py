from dash import Dash, html, dcc
from dash.dependencies import Input, Output
import plotly.express as px
import pandas as pd
import duckdb
from datetime import datetime, timedelta

app = Dash(__name__)
DB_PATH = 'data/processed_weather.duckdb'

def get_weather_data():
    with duckdb.connect(DB_PATH) as conn:
        df = conn.execute("""
            SELECT 
                timestamp,
                temperature,
                humidity,
                wind_speed,
                wind_direction,
                rain,
                rain_delta,
                light_lux,
                delta_t
            FROM processed_intervals
            ORDER BY timestamp DESC
            LIMIT 288  -- 24 hours of 5-min intervals
        """).fetchdf()
    return df

app.layout = html.Div([
    html.H1('Weather Station Dashboard'),
    dcc.Interval(id='interval-component', interval=5*60*1000, n_intervals=0),
    dcc.DatePickerRange(
        id='date-picker',
        start_date=datetime.now().date(),
        end_date=datetime.now().date(),
    ),
    html.Div([
        dcc.Graph(id='temperature-graph'),
        dcc.Graph(id='humidity-graph'),
        dcc.Graph(id='wind-graph'),
        dcc.Graph(id='rain-graph'),
        dcc.Graph(id='light-graph')
    ])
])

@app.callback(
    [Output('temperature-graph', 'figure'),
     Output('humidity-graph', 'figure'),
     Output('wind-graph', 'figure'),
     Output('rain-graph', 'figure'),
     Output('light-graph', 'figure')],
    [Input('interval-component', 'n_intervals'),
     Input('date-picker', 'start_date'),
     Input('date-picker', 'end_date')]
)
def update_graphs(n, start_date, end_date):
    df = get_weather_data()
    
    temp_fig = px.line(df, x='timestamp', y='temperature', 
                       title='Temperature (°C)')
    
    humidity_fig = px.line(df, x='timestamp', y='humidity',
                          title='Humidity (%)')
    
    wind_fig = px.scatter_polar(df, r='wind_speed', theta='wind_direction',
                               title='Wind Speed and Direction')
    
    rain_fig = px.bar(df, x='timestamp', y='rain_delta',
                      title=' 5 min Rain (mm)')
    
    light_fig = px.line(df, x='timestamp', y='light_lux',
                        title='Light Intensity (lux)')
    
    return temp_fig, humidity_fig, wind_fig, rain_fig, light_fig

if __name__ == '__main__':
    app.run_server(debug=True,
                   host='0.0.0.0',
                   port=8050)