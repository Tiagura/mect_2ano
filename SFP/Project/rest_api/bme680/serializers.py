from rest_framework import serializers
from .models import BME680

class BME680Serializer(serializers.ModelSerializer):
    class Meta:
        model = BME680
        fields = '__all__'

    def create(self, validated_data):
        return BME680.objects.create(**validated_data)
        
    def save(self):
        return super().save()
    
    def to_representation(self, instance):
        return {
            'id': instance.id,
            'temperature': instance.temperature,
            'humidity': instance.humidity,
            'pressure': instance.pressure,
            'gas': instance.gas,
            'time': instance.time
        }
    
    def serialize(self):
        return super().serialize()
        
