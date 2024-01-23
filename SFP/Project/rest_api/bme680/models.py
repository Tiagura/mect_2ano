from django.db import models

class BME680(models.Model):
    id = models.AutoField(primary_key=True)
    temperature = models.FloatField()
    humidity = models.FloatField()
    pressure = models.FloatField()
    gas = models.FloatField()
    time = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f'{self.id} {self.temperature} {self.pressure} {self.humidity} {self.gas} {self.time}'

    class Meta:
        verbose_name = 'BME680'
        verbose_name_plural = 'BME680'
        ordering = ['id']