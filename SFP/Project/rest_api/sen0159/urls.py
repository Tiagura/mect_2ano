from django.urls import path
from .views import (
    real_time,
    history,
)

app_name = 'sen0159'

urlpatterns = [ path("", real_time, name="real_time"),
                path("history/<int:last_of>", history, name="history"),
                ]