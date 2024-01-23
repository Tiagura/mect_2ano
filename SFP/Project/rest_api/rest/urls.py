"""
URL configuration for rest project.

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/4.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path, include

from .views import get_logs

urlpatterns = [
    path('admin/', admin.site.urls),
    path("bme680/", include("bme680.urls", "bme680")),
    path("mq9/", include("mq9.urls", "mq9")),
    path("sen0159/", include("sen0159.urls", "sen0159")),
    path("logs/", get_logs, name="logs"),
]
